export const mediaController = {
  execute(action: string, element: HTMLMediaElement | null) {
    if (!element) return;

    switch (action) {
      case 'PlayPause':
        if (element.paused) {
          element.play().catch(console.error);
        } else {
          element.pause();
        }
        break;
      case 'Next':
        // Try Media Session action first
        if ('mediaSession' in navigator) {
            // @ts-ignore
            navigator.mediaSession.setActionHandler('nexttrack', null); 
            // Note: triggering media session actions programmatically from content script is hard.
            // We usually simulate key presses or click next buttons if we know the site.
            // For generic implementation, we might just skip forward.
            element.currentTime += 10; 
        }
        break;
      case 'Prev':
        element.currentTime -= 10;
        break;
      case 'TogglePip':
        if (document.pictureInPictureElement) {
          document.exitPictureInPicture().catch(console.error);
        } else if (element instanceof HTMLVideoElement) {
          element.requestPictureInPicture().catch(console.error);
        }
        break;
    }
  }
};
