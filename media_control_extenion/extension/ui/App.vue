<script setup lang="ts">
import { ref, onMounted, onUnmounted, computed } from 'vue';
import type { HubState } from '../shared/types';

const state = ref<HubState>({
  connection: { connected: false },
  media: { activeTabId: null, byTabId: {} },
  ui: { pinnedTabId: null, lastFocusedTabId: undefined }
});

const formatArtist = (urlOrArtist?: string) => {
  if (!urlOrArtist) return 'Unknown Site';
  if (urlOrArtist.includes('.')) {
    let domain = urlOrArtist.replace(/^www\./, '');
    const parts = domain.split('.');
    if (parts.length > 1) {
       parts.pop();
       return parts.join('.');
    }
    return domain;
  }
  return urlOrArtist;
};

const mediaList = computed(() => {
  const list = Object.entries(state.value.media.byTabId).map(([id, s]) => ({
    id: parseInt(id),
    state: s
  }));
  
  const activeId = state.value.media.activeTabId;
  
  list.sort((a, b) => {
    if (a.id === activeId) return -1;
    if (b.id === activeId) return 1;
    if (a.state.playing && !b.state.playing) return -1;
    if (!a.state.playing && b.state.playing) return 1;
    return b.state.lastActiveAt - a.state.lastActiveAt;
  });
  
  return list;
});

const handleMessage = (msg: any) => {
  if (msg.target === 'ui' && msg.action === 'stateUpdate') {
    state.value = msg.payload;
  }
};

const handleImageError = (e: Event) => {
    const target = e.target as HTMLImageElement;
    target.style.display = 'none';
    if (target.nextElementSibling) {
        (target.nextElementSibling as HTMLElement).style.display = 'flex';
    }
};

onMounted(() => {
  chrome.runtime.onMessage.addListener(handleMessage);
  chrome.runtime.sendMessage({ target: 'sw', action: 'getState' }, (response) => {
    if (response) state.value = response;
  });
});

onUnmounted(() => {
  chrome.runtime.onMessage.removeListener(handleMessage);
});

const sendCmd = (name: string, tabId?: number) => {
  chrome.runtime.sendMessage({
    target: 'sw',
    action: 'cmd',
    payload: { type: 'cmd', name, payload: { tabId } }
  });
};

const activateTab = (item: any) => {
  chrome.runtime.sendMessage({
    target: 'sw',
    action: 'activateTab',
    payload: { tabId: item.id, windowId: item.state.windowId }
  });
};

const closeTab = (tabId: number) => {
  chrome.runtime.sendMessage({
    target: 'sw',
    action: 'closeTab',
    payload: { tabId }
  });
};

const connectNative = () => {
  // Disconnect first to avoid spawning duplicate host processes
  chrome.runtime.sendMessage({ target: 'sw', action: 'disconnectNative' }, () => {
    chrome.runtime.sendMessage({ target: 'sw', action: 'connectNative' });
  });
};
</script>

<template>
  <div class="app-root">
    <header>
      <h1>Media Hub</h1>
      <div class="status" :class="{ connected: state.connection.connected }">
        <span class="indicator"></span>
        {{ state.connection.connected ? 'Connected' : 'Offline' }}
        <button v-if="!state.connection.connected" @click="connectNative" class="connect-btn">Retry</button>
      </div>
    </header>

    <main>
      <div v-if="mediaList.length === 0" class="empty">
        <div class="empty-icon">🎵</div>
        <p>No media playing</p>
      </div>

      <div 
        v-for="item in mediaList" 
        :key="item.id" 
        class="card" 
        :class="{ playing: item.state.playing }"
        @click="activateTab(item)"
      >
         <!-- Color Extraction Background (Tint) -->
         <div 
            class="card-tint" 
            :style="{ backgroundImage: `url(${item.state.artwork})` }"
         ></div>

         <div class="card-content">
             <div class="cover">
                <img 
                    v-if="item.state.artwork" 
                    :src="item.state.artwork" 
                    @error="handleImageError"
                />
                <div class="cover-placeholder" :style="{ display: item.state.artwork ? 'none' : 'flex' }">
                   <span>♪</span>
                </div>
             </div>
             
             <div class="content-wrapper">
                 <div class="top-row">
                     <div class="info">
                        <h3 :title="item.state.title">{{ item.state.title || 'Unknown Title' }}</h3>
                        <p>{{ formatArtist(item.state.artist) }}</p>
                     </div>
                     <button class="close-btn" @click.stop="closeTab(item.id)" title="Close Tab">×</button>
                 </div>
                 
                 <div class="bottom-row">
                     <div class="controls">
                        <button @click.stop="sendCmd('Prev', item.id)">⏮</button>
                        <button @click.stop="sendCmd('PlayPause', item.id)" class="play-btn">
                          {{ item.state.playing ? '⏸' : '▶' }}
                        </button>
                        <button @click.stop="sendCmd('Next', item.id)">⏭</button>
                     </div>
                     <button class="pip-btn" @click.stop="sendCmd('TogglePip', item.id)" title="Picture-in-Picture">
                        ⤢
                     </button>
                 </div>
             </div>
         </div>
         
         <!-- Explicit DOM Element for Playing Indicator (Z-Index 100) -->
         <div v-if="item.state.playing" class="playing-bar"></div>
      </div>
    </main>
  </div>
</template>

<style scoped>
/* Reset & Base */
:root {
  --bg-color: #f2f2f7;
  --text-primary: #000;
  --text-secondary: #666;
  --card-bg-fallback: #fff;
  --btn-bg: rgba(0,0,0,0.05);
  --btn-hover: rgba(0,0,0,0.1);
  --accent: #34c759;
  
  /* Stronger borders for visibility against white backgrounds */
  --card-border: rgba(0,0,0,0.12);
  /* Deeper shadow for better layering */
  --card-shadow: 0 2px 8px rgba(0,0,0,0.06), 0 1px 2px rgba(0,0,0,0.04);
  --card-shadow-hover: 0 12px 24px rgba(0,0,0,0.15), 0 4px 8px rgba(0,0,0,0.08);
}

@media (prefers-color-scheme: dark) {
  :root {
    --bg-color: #000;
    --text-primary: #fff;
    --text-secondary: #aaa;
    --card-bg-fallback: #1c1c1e;
    --btn-bg: rgba(255,255,255,0.1);
    --btn-hover: rgba(255,255,255,0.2);
    --accent: #30d158;
    --card-border: rgba(255,255,255,0.15);
    --card-shadow: 0 4px 12px rgba(0,0,0,0.4);
    --card-shadow-hover: 0 8px 24px rgba(0,0,0,0.6);
  }
}

.app-root {
  width: 360px;
  min-height: 200px;
  padding: 16px;
  background-color: var(--bg-color);
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
}

header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 16px;
}

h1 {
  font-size: 18px;
  font-weight: 700;
  margin: 0;
  color: var(--text-primary);
}

.status {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 12px;
  color: var(--text-secondary);
}

.indicator {
  width: 6px;
  height: 6px;
  border-radius: 50%;
  background-color: #8e8e93;
}

.status.connected .indicator {
  background-color: var(--accent);
}

.connect-btn {
    font-size: 10px;
    padding: 2px 8px;
    background: var(--btn-bg);
    color: var(--text-primary);
    border-radius: 4px;
}

.empty {
    text-align: center;
    padding: 40px 0;
    color: var(--text-secondary);
}
.empty-icon {
    font-size: 48px;
    opacity: 0.3;
    margin-bottom: 8px;
}

/* Card Styles */
.card {
  position: relative;
  height: 100px;
  border-radius: 16px;
  margin-bottom: 12px;
  overflow: hidden; /* Clips content but playing-bar is absolute inside */
  cursor: pointer;
  transition: transform 0.2s cubic-bezier(0.25, 0.8, 0.25, 1), box-shadow 0.2s;
  background-color: var(--card-bg-fallback);
  
  /* Visible Border for Layering */
  border: 1px solid var(--card-border);
  box-shadow: var(--card-shadow);
  
  isolation: isolate;
}

.card:hover {
  transform: scale(1.02);
  z-index: 10;
  box-shadow: var(--card-shadow-hover);
  border-color: rgba(0,0,0,0.2); /* Darker border on hover */
}

@media (prefers-color-scheme: dark) {
    .card:hover {
        border-color: rgba(255,255,255,0.3);
    }
}

/* Dynamic Color Background (Tint) */
.card-tint {
  position: absolute;
  top: 0; left: 0; right: 0; bottom: 0;
  background-size: cover;
  background-position: center;
  filter: blur(40px) saturate(1.5);
  opacity: 0.25; /* Subtle tint */
  z-index: 0;
}

/* Card Content Layout */
.card-content {
  position: relative;
  z-index: 1;
  display: flex;
  height: 100%;
  background: transparent;
}

.cover {
  width: 100px;
  height: 100%;
  flex-shrink: 0;
  background: #333;
  position: relative;
  /* Separate cover from content with a subtle border if needed */
  border-right: 1px solid var(--card-border);
}

.cover img {
  width: 100%;
  height: 100%;
  object-fit: cover;
  display: block;
}

.cover-placeholder {
  width: 100%;
  height: 100%;
  display: flex;
  align-items: center;
  justify-content: center;
  background: #eee;
  color: #aaa;
  font-size: 32px;
}

.content-wrapper {
  flex: 1;
  display: flex;
  flex-direction: column;
  justify-content: space-between;
  padding: 12px;
  min-width: 0;
  padding-right: 16px; 
}

.top-row {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  gap: 8px;
}

.info {
  flex: 1;
  min-width: 0;
}

.info h3 {
  font-size: 15px;
  line-height: 1.2;
  margin: 0 0 4px 0;
  font-weight: 600;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  color: var(--text-primary);
}

.info p {
  font-size: 13px;
  color: var(--text-secondary);
  margin: 0;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.close-btn {
  background: transparent;
  color: var(--text-secondary);
  font-size: 18px;
  width: 24px;
  height: 24px;
  padding: 0;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: 50%;
  margin-right: -4px;
  margin-top: -4px;
  border: none;
  cursor: pointer;
}
.close-btn:hover {
  background: rgba(255, 59, 48, 0.15);
  color: #ff3b30;
}

.bottom-row {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.controls {
  display: flex;
  gap: 12px;
}

button {
  border: none;
  background: var(--btn-bg);
  color: var(--text-primary);
  width: 32px;
  height: 32px;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  cursor: pointer;
  transition: background 0.2s, transform 0.1s;
  font-size: 16px;
}

button:hover {
  background: var(--btn-hover);
  transform: scale(1.05);
}

.play-btn {
  font-size: 14px;
  background: var(--text-primary);
  color: var(--bg-color); /* Invert colors for play btn */
}

.play-btn:hover {
  opacity: 0.9;
  background: var(--text-primary);
}

.pip-btn {
  font-size: 18px;
  background: transparent;
  color: var(--text-secondary);
}

.pip-btn:hover {
  background: var(--btn-bg);
  color: var(--text-primary);
}

/* Explicit Playing Bar */
.playing-bar {
  position: absolute;
  top: 0;
  bottom: 0;
  right: 0;
  width: 6px;
  background: var(--accent);
  z-index: 100; /* Ensure it's on top of everything */
  box-shadow: 0 0 12px var(--accent);
  animation: pulse 2s infinite;
}

@keyframes pulse {
  0% { opacity: 0.6; }
  50% { opacity: 1; box-shadow: 0 0 16px var(--accent); }
  100% { opacity: 0.6; }
}
</style>
