export interface MediaState {
  hasMedia: boolean;
  playing: boolean;
  title?: string;
  artist?: string;
  album?: string;
  artwork?: string;
  durationMs?: number;
  positionMs?: number;
  lastActiveAt: number;
  isInteraction?: boolean; // True if update triggered by play/pause or new media load
  windowId?: number;
}

export interface HubState {
  connection: {
    connected: boolean;
    lastError?: string;
  };
  media: {
    activeTabId: number | null;
    byTabId: Record<number, MediaState>;
  };
  ui: {
    pinnedTabId: number | null;
    lastFocusedTabId?: number;
  };
}
