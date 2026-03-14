export const mediaDetector = {
  element: null as HTMLMediaElement | null,
  _intervalId: null as ReturnType<typeof setInterval> | null,
  _eventCleanup: null as (() => void) | null,

  find() {
    const mediaElements = Array.from(document.querySelectorAll('video, audio')) as HTMLMediaElement[];

    if (mediaElements.length === 0) return null;

    const validElements = mediaElements.filter(el => {
      // 1. Ignore empty tags used as placeholders (no src attribute, no currentSrc, no <source> children)
      const hasSrc = el.hasAttribute('src') || el.currentSrc || el.querySelector('source');
      if (!hasSrc) {
        return false;
      }

      // 2. Ignore ambient background videos (usually autoplay + loop + muted)
      const isAutoPlay = el.autoplay || el.hasAttribute('autoplay') || el.hasAttribute('autoPlay');
      const isLoop = el.loop || el.hasAttribute('loop');
      const isMuted = el.muted || el.hasAttribute('muted');
      const isPlaysInline = el.hasAttribute('playsinline') || el.hasAttribute('playsInline');

      if (el.tagName === 'VIDEO' && isAutoPlay && isLoop && isMuted) {
        return false;
      }

      // 2b. playsInline + muted is almost certainly decorative UI
      if (el.tagName === 'VIDEO' && isPlaysInline && isMuted) {
         return false;
      }

      // 2c. Ignore invisible or tiny video elements (1x1 tracking pixels, hidden preloaders)
      // Only for VIDEO — audio elements legitimately have 0x0 dimensions (custom UI players)
      if (el.tagName === 'VIDEO' && el.isConnected) {
         const rect = el.getBoundingClientRect();
         if (rect.width * rect.height <= 4) {
             return false;
         }

         // Fullscreen background wallpaper video (>80% viewport, no controls)
         if (!el.hasAttribute('controls')) {
             const vw = Math.max(document.documentElement.clientWidth || 0, window.innerWidth || 0);
             const vh = Math.max(document.documentElement.clientHeight || 0, window.innerHeight || 0);
             if (rect.width > vw * 0.8 && rect.height > vh * 0.8) {
                 return false;
             }
         }
      }

      // 3. Ignore tiny notification sounds (< 3s)
      if (el.readyState >= 1 && el.duration < 3) {
        return false;
      }

      return true;
    });

    if (validElements.length === 0) return null;

    // Prioritize playing elements
    const playing = validElements.find(el => !el.paused);
    if (playing) return playing;

    // Fallback to the first one
    return validElements[0];
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
        // Notify SW that media is gone before unbinding
        if (!el && this.element) {
          onChange({ hasMedia: false, playing: false });
        }
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
