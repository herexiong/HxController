export const mediaDetector = {
  element: null as HTMLMediaElement | null,
  _intervalId: null as ReturnType<typeof setInterval> | null,
  _eventCleanup: null as (() => void) | null,

  find() {
    const mediaElements = Array.from(document.querySelectorAll('video, audio'));
    
    if (mediaElements.length === 0) return null;

    // Prioritize playing elements
    const playing = mediaElements.find(el => !(el as HTMLMediaElement).paused);
    if (playing) return playing as HTMLMediaElement;

    // Fallback to the first one
    return mediaElements[0] as HTMLMediaElement;
  },

  start(onChange: (state: any) => void) {
    // Clean up previous run if any
    this.stop();

    const check = () => {
      // Check if extension context is still valid
      if (!chrome.runtime?.id) {
        this.stop();
        return;
      }

      const el = this.find();
      if (el !== this.element) {
        this.bind(el, onChange);
      } else if (el) {
        // Periodically update metadata in case title changed (SPA navigation)
        onChange(this.getState(false));
      }
    };
    
    this._intervalId = setInterval(check, 2000);
    check();
  },

  stop() {
    if (this._intervalId) {
      clearInterval(this._intervalId);
      this._intervalId = null;
    }
    if (this._eventCleanup) {
      this._eventCleanup();
      this._eventCleanup = null;
    }
    this.element = null;
  },

  bind(el: HTMLMediaElement | null, onChange: (state: any) => void) {
    // Remove old listeners via closure reference
    if (this._eventCleanup) {
      this._eventCleanup();
      this._eventCleanup = null;
    }

    this.element = el;

    if (el) {
      console.log('Media element found:', el);
      
      // Create listeners as local variables — ensures correct removal
      const onPlay = () => onChange(this.getState(true));
      const onPause = () => onChange(this.getState(true));
      
      let lastUpdate = 0;
      const onTimeUpdate = () => {
        const now = Date.now();
        if (now - lastUpdate > 1000) {
          onChange(this.getState(false)); // Time update is NOT an interaction
          lastUpdate = now;
        }
      };

      el.addEventListener('play', onPlay);
      el.addEventListener('pause', onPause);
      el.addEventListener('timeupdate', onTimeUpdate);

      // Store cleanup closure for exact same references
      this._eventCleanup = () => {
        el.removeEventListener('play', onPlay);
        el.removeEventListener('pause', onPause);
        el.removeEventListener('timeupdate', onTimeUpdate);
      };
      
      // Initial state
      onChange(this.getState(true));
    }
  },

  getState(isInteraction: boolean = false) {
    if (!this.element) return { hasMedia: false, playing: false };

    let title = document.title;
    let artist = window.location.hostname;
    let artwork = '';

    const makeAbsolute = (url: string) => {
        if (!url) return '';
        try {
            return new URL(url, document.baseURI).href;
        } catch {
            return url;
        }
    };

    // 1. Try generic Open Graph image
    const ogImage = document.querySelector('meta[property="og:image"]');
    if (ogImage) {
      artwork = makeAbsolute(ogImage.getAttribute('content') || '');
    }

    // 2. Try video poster
    if (!artwork && this.element instanceof HTMLVideoElement && this.element.poster) {
      artwork = makeAbsolute(this.element.poster);
    }

    // 3. Site-specific overrides
    if (window.location.hostname.includes('bilibili.com')) {
      const bTitle = document.querySelector('.video-info-title, h1.video-title, .video-title');
      if (bTitle) {
        title = (bTitle as HTMLElement).innerText;
      }
      // Bilibili cover
      const bCover = document.querySelector('meta[itemprop="image"]');
      if (bCover) {
        artwork = makeAbsolute(bCover.getAttribute('content') || artwork);
      }
    }

    // 4. Media Session API (Highest priority)
    if ('mediaSession' in navigator && navigator.mediaSession.metadata) {
      const meta = navigator.mediaSession.metadata;
      if (meta.title) title = meta.title;
      if (meta.artist) artist = meta.artist;
      if (meta.artwork && meta.artwork.length > 0) {
        // Find largest artwork
        const art = meta.artwork.reduce((prev, curr) => {
          const pSize = parseInt(prev.sizes?.split('x')[0] || '0');
          const cSize = parseInt(curr.sizes?.split('x')[0] || '0');
          return cSize > pSize ? curr : prev;
        });
        artwork = makeAbsolute(art.src);
      }
    }
    
    // If in iframe and title is generic/empty, clear it so SW can fill with Tab title
    if (window.self !== window.top) {
        if (!title || title === 'about:blank' || title.startsWith('http')) {
            title = ''; 
        }
    }

    return {
      hasMedia: true,
      playing: !this.element.paused,
      durationMs: this.element.duration * 1000,
      positionMs: this.element.currentTime * 1000,
      lastActiveAt: Date.now(),
      isInteraction, // Flag to indicate if this update was caused by user interaction/state change
      title,
      artist,
      artwork
    };
  }
};
