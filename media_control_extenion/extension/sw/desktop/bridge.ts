import type { Envelope } from '../../shared/protocol';
import { hubStore } from '../hub/store';

const HOST_NAME = 'com.hxmonitor.proxy';

let port: chrome.runtime.Port | null = null;

export const desktopBridge = {
  connect() {
    // Disconnect existing connection first
    if (port) {
      this.disconnect();
    }

    try {
      port = chrome.runtime.connectNative(HOST_NAME);

      // Use arrow function — always delegates to current this.handleMessage
      port.onMessage.addListener((msg: Envelope) => {
        this.onPortMessage(msg);
      });

      port.onDisconnect.addListener(() => {
        console.log('Native Messaging disconnected:', chrome.runtime.lastError?.message);
        port = null;
        hubStore.setConnectionState(false, chrome.runtime.lastError?.message || 'Disconnected');
      });

      hubStore.setConnectionState(true);
      console.log('Native Messaging connected');
    } catch (err: any) {
      console.error('Failed to connect to native host:', err);
      hubStore.setConnectionState(false, err.message);
    }
  },

  disconnect() {
    if (port) {
      port.disconnect();
      port = null;
      hubStore.setConnectionState(false, 'Disconnected by user');
    }
  },

  isConnected(): boolean {
    return port !== null;
  },

  /**
   * Send an event or response to the desktop host.
   * Used for evt (push) and res (response) messages.
   */
  send(msg: Envelope) {
    if (port) {
      port.postMessage(msg);
    }
  },

  /**
   * Internal: handle incoming messages from native port.
   * Separates responses (res) from commands/events, then delegates.
   */
  onPortMessage(msg: Envelope) {
    // Delegate all incoming messages to the injected handler
    this.handleMessage(msg);
  },

  /**
   * Injected by sw/index.ts to route messages to the router.
   * Default: no-op. Will be overwritten at startup.
   */
  handleMessage(_msg: Envelope) {
    // Stub — replaced by sw/index.ts
  }
};
