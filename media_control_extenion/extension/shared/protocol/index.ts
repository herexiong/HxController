import type { MediaState } from '../types';

// --- Message Envelope ---

export type MessageType = 'cmd' | 'res' | 'evt';

export type CmdName = 'PlayPause' | 'Next' | 'Prev' | 'TogglePip' | 'GetMediaList';
export type EvtName = 'MediaList' | 'ActiveTabChanged' | 'HostReady';

export interface Envelope<T = any> {
  type: MessageType;
  name: string;
  id?: number;       // Required for cmd, optional for evt
  replyTo?: number;  // Required for res
  ts?: number;
  payload: T;
}

// --- Command Payloads (Desktop -> Extension) ---

export interface CmdPayload {
  tabId?: number;  // If omitted, uses activeMediaTabId
}

// GetMediaList has no payload fields
export type GetMediaListPayload = Record<string, never>;

// --- Response Payloads (Extension -> Desktop) ---

export interface CmdResponse {
  ok: boolean;
  error?: string;
}

// --- Event Payloads (Extension -> Desktop) ---

export interface MediaListPayload {
  activeTabId: number | null;
  tabs: Record<number, MediaState>;
}

export interface ActiveTabChangedPayload {
  activeTabId: number | null;
}

export interface HostReadyPayload {
  version: string;
}

// --- Internal Runtime Messages (Extension internal use only) ---

export interface RuntimeMessage {
  target: 'sw' | 'content' | 'ui';
  action: string;
  payload?: any;
}
