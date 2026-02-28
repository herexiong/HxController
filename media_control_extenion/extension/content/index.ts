import { mediaDetector } from './detect/media';
import { mediaController } from './actions/control';

console.log('Content Script loaded');

// Helper to safely send messages
const safeSendMessage = async (msg: any) => {
  if (!chrome.runtime?.id) return;
  try {
    await chrome.runtime.sendMessage(msg);
  } catch (err: any) {
    // Ignore context invalidated errors, but log others
    if (err.message && !err.message.includes('Extension context invalidated')) {
      // console.warn('Msg send failed:', err);
    }
  }
};

// Start detection
mediaDetector.start((state) => {
  safeSendMessage({
    target: 'sw',
    action: 'stateUpdate',
    payload: state
  });
});

// Listen for commands from SW
chrome.runtime.onMessage.addListener((msg) => {
  if (msg.target === 'content') {
    if (msg.action === 'execute') {
      const cmd = msg.payload;
      console.log('Executing command:', cmd.name);
      mediaController.execute(cmd.name, mediaDetector.element);
    } else if (msg.action === 'queryState') {
      // Force update state
      const state = mediaDetector.getState(false);
      if (state.hasMedia) {
        safeSendMessage({
          target: 'sw',
          action: 'stateUpdate',
          payload: state
        });
      }
    }
  }
});
