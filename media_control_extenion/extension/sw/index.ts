import { router } from './hub/router';
import { desktopBridge } from './desktop/bridge';

console.log('Hub Core (SW) started');

// Setup Desktop Bridge Listener
// Inject handleMessage dependency
desktopBridge.handleMessage = (msg) => router.handleDesktopMessage(msg);

// Setup Runtime Message Listener
chrome.runtime.onMessage.addListener((message, sender, sendResponse) => {
  router.handleRuntimeMessage(message, sender, sendResponse);
  // Return true to allow async sendResponse (even if we use it synchronously now, it's safer for future)
  return true;
});

// Handle Tab Removal
chrome.tabs.onRemoved.addListener((tabId) => {
  import('./hub/store').then(({ hubStore }) => {
    hubStore.removeTab(tabId);
  });
});

// Handle Tab Activation (Focus)
chrome.tabs.onActivated.addListener((activeInfo) => {
  import('./hub/store').then(({ hubStore }) => {
    hubStore.setFocusedTab(activeInfo.tabId);
  });
});

// Auto-connect to Desktop Host on startup
desktopBridge.connect();
