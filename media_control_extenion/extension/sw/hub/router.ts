import { hubStore } from './store';
import { desktopBridge } from '../desktop/bridge';
import type { Envelope, RuntimeMessage } from '../../shared/protocol';

export const router = {
  // Handle messages from Desktop (Native Messaging)
  handleDesktopMessage(msg: Envelope) {
    if (msg.type === 'cmd') {
      // GetMediaList — return state snapshot directly, no tab command needed
      if (msg.name === 'GetMediaList') {
        const state = hubStore.getState();
        const hasTabs = Object.keys(state.media.byTabId).length > 0;

        if (hasTabs) {
          // We have state — respond immediately
          if (msg.id != null) {
            desktopBridge.send({
              type: 'res',
              name: 'GetMediaList',
              replyTo: msg.id,
              ts: Date.now(),
              payload: {
                ok: true,
                activeTabId: state.media.activeTabId,
                tabs: state.media.byTabId
              }
            });
          }
        } else {
          // SW has no state yet (just restarted) — query all tabs, then respond
          chrome.tabs.query({}, (tabs) => {
            for (const tab of tabs) {
              if (tab.id) {
                chrome.tabs.sendMessage(tab.id, {
                  target: 'content',
                  action: 'queryState'
                }).catch(() => { /* tab has no content script */ });
              }
            }
          });

          // Wait for content scripts to respond, then push MediaList
          const replyId = msg.id;
          setTimeout(() => {
            const freshState = hubStore.getState();
            if (replyId != null) {
              desktopBridge.send({
                type: 'res',
                name: 'GetMediaList',
                replyTo: replyId,
                ts: Date.now(),
                payload: {
                  ok: true,
                  activeTabId: freshState.media.activeTabId,
                  tabs: freshState.media.byTabId
                }
              });
            }
            // Also push as evt so desktop updates even if it ignores the res
            hubStore.notifyDesktop();
          }, 600);
        }
        return;
      }

      // All other commands: forward to content script
      this.executeCommand(msg).then((ok) => {
        if (msg.id != null) {
          desktopBridge.send({
            type: 'res',
            name: msg.name,
            replyTo: msg.id,
            ts: Date.now(),
            payload: { ok }
          });
        }
      });
    }
  },

  // Handle messages from UI/Content (Runtime Messaging)
  handleRuntimeMessage(msg: RuntimeMessage, sender: chrome.runtime.MessageSender, sendResponse?: (response: any) => void) {
    if (msg.target === 'sw') {
      if (msg.action === 'stateUpdate' && sender.tab?.id) {
        // Content Script reporting state
        const mediaState = msg.payload;

        // Fallback for missing title (especially from iframes)
        if (!mediaState.title && sender.tab.title) {
          mediaState.title = sender.tab.title;
        }

        // If URL/Artist is missing, use tab url hostname
        if (!mediaState.artist && sender.tab.url) {
          try {
            mediaState.artist = new URL(sender.tab.url).hostname;
          } catch (e) { }
        }

        // Inject windowId
        if (sender.tab.windowId) {
          mediaState.windowId = sender.tab.windowId;
        }

        // Fallback for artwork using Favicon
        if (!mediaState.artwork && sender.tab.favIconUrl) {
          mediaState.artwork = sender.tab.favIconUrl;
        }

        hubStore.updateMediaState(sender.tab.id, mediaState);
      } else if (msg.action === 'connectNative') {
        desktopBridge.connect();
      } else if (msg.action === 'disconnectNative') {
        desktopBridge.disconnect();
      } else if (msg.action === 'cmd') {
        // UI sending command
        this.executeCommand(msg.payload);
      } else if (msg.action === 'getState') {
        const state = hubStore.getState();
        if (sendResponse) {
          sendResponse(state);
        }

        // If no media is known (SW restart), broadcast query to all tabs
        if (Object.keys(state.media.byTabId).length === 0) {
          chrome.tabs.query({}, (tabs) => {
            for (const tab of tabs) {
              if (tab.id) {
                chrome.tabs.sendMessage(tab.id, {
                  target: 'content',
                  action: 'queryState'
                }).catch(() => {
                  // Ignore errors (e.g. tab has no content script)
                });
              }
            }
          });
        }
      } else if (msg.action === 'closeTab') {
        if (msg.payload?.tabId) {
          chrome.tabs.remove(msg.payload.tabId);
        }
      } else if (msg.action === 'activateTab') {
        if (msg.payload?.tabId) {
          chrome.tabs.update(msg.payload.tabId, { active: true });
          // Only focus window if windowId is provided
          if (msg.payload.windowId) {
            chrome.windows.update(msg.payload.windowId, { focused: true });
          }
        }
      }
    }
  },

  async executeCommand(cmd: Envelope): Promise<boolean> {
    const targetTabId = cmd.payload?.tabId || hubStore.getState().media.activeTabId;

    if (!targetTabId) {
      console.warn('No active media tab found for command:', cmd.name);
      return false;
    }

    try {
      await chrome.tabs.sendMessage(targetTabId, {
        target: 'content',
        action: 'execute',
        payload: cmd
      });
      return true;
    } catch (err) {
      console.error('Failed to send command to tab:', targetTabId, err);
      // Clean up dead tab
      hubStore.removeTab(targetTabId);
      return false;
    }
  }
};
