import type { HubState, MediaState } from '../../shared/types';
import { desktopBridge } from '../desktop/bridge';

const state: HubState = {
  connection: {
    connected: false,
    lastError: undefined,
  },
  media: {
    activeTabId: null,
    byTabId: {},
  },
  ui: {
    pinnedTabId: null,
    lastFocusedTabId: undefined,
  },
};

let _desktopNotifyTimer: ReturnType<typeof setTimeout> | null = null;

export const hubStore = {
  getState() {
    return state;
  },

  setConnectionState(connected: boolean, error?: string) {
    state.connection.connected = connected;
    state.connection.lastError = error;
    this.notifyUI();
    // Push current media list to desktop immediately after connecting
    if (connected) {
      this.notifyDesktop();
    }
  },

  setFocusedTab(tabId: number) {
    state.ui.lastFocusedTabId = tabId;
    this.recomputeActiveTab();
    this.notifyUI();
  },

  updateMediaState(tabId: number, mediaState: MediaState) {
    // If media disappeared from this tab, treat it as a removal
    if (!mediaState.hasMedia) {
      this.removeTab(tabId);
      return;
    }

    const existing = state.media.byTabId[tabId];
    if (existing) {
      if (!mediaState.isInteraction && existing.playing === mediaState.playing) {
        mediaState.lastActiveAt = existing.lastActiveAt;
      }
    }

    state.media.byTabId[tabId] = mediaState;
    this.recomputeActiveTab();
    this.notifyUI();
    this.notifyDesktop();
  },

  removeTab(tabId: number) {
    if (state.media.byTabId[tabId]) {
      delete state.media.byTabId[tabId];
      this.recomputeActiveTab();
      this.notifyUI();
      this.notifyDesktop();
    }
  },

  recomputeActiveTab() {
    // Logic:
    // 1. If lastFocusedTabId has media, it's the active one (user is looking at it)
    // 2. Else, fallback to playing/recent logic

    if (state.ui.lastFocusedTabId && state.media.byTabId[state.ui.lastFocusedTabId]) {
      state.media.activeTabId = state.ui.lastFocusedTabId;
      return;
    }

    const tabs = Object.entries(state.media.byTabId).map(([id, s]) => ({
      id: parseInt(id),
      ...s
    }));

    if (tabs.length === 0) {
      state.media.activeTabId = null;
      return;
    }

    tabs.sort((a, b) => {
      if (a.playing && !b.playing) return -1;
      if (!a.playing && b.playing) return 1;
      return b.lastActiveAt - a.lastActiveAt;
    });

    state.media.activeTabId = tabs[0].id;
  },

  /**
   * Push state to Popup UI via runtime message.
   */
  notifyUI() {
    chrome.runtime.sendMessage({
      target: 'ui',
      action: 'stateUpdate',
      payload: state
    }).catch(() => { });
  },

  /**
   * Push MediaList to desktop host via Native Messaging (debounced 100ms).
   */
  notifyDesktop() {
    if (!desktopBridge.isConnected()) return;
    if (_desktopNotifyTimer) return;

    _desktopNotifyTimer = setTimeout(() => {
      _desktopNotifyTimer = null;
      desktopBridge.send({
        type: 'evt',
        name: 'MediaList',
        ts: Date.now(),
        payload: {
          activeTabId: state.media.activeTabId,
          tabs: state.media.byTabId
        }
      });
    }, 100);
  }
};
