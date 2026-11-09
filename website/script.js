const API_BASE = '/api/v1';
const params = new URLSearchParams(window.location.search);
const doorId = params.get('door') || 'door-001';

const doorName = document.querySelector('#door-name');
const doorIdLabel = document.querySelector('#door-id');
const doorState = document.querySelector('#door-state');
const usernameInput = document.querySelector('#username');
const authenticateButton = document.querySelector('#authenticate-button');
const registerButton = document.querySelector('#register-button');
const lockButton = document.querySelector('#lock-button');
const lockPanel = document.querySelector('#lock-panel');
const statusMessage = document.querySelector('#status-message');

doorIdLabel.textContent = `DOOR ID · ${doorId.toUpperCase()}`;

function setStatus(message, kind = '') {
  statusMessage.textContent = message;
  statusMessage.className = `status-message${kind ? ` is-${kind}` : ''}`;
}

function setBusy(busy, activeButton) {
  [authenticateButton, registerButton, lockButton].forEach((button) => {
    if (button) button.disabled = busy;
  });
  if (activeButton && busy) activeButton.dataset.originalText = activeButton.textContent;
  if (activeButton && !busy && activeButton.dataset.originalText) {
    activeButton.textContent = activeButton.dataset.originalText;
    delete activeButton.dataset.originalText;
  }
}

function toBase64Url(buffer) {
  const bytes = new Uint8Array(buffer);
  let binary = '';
  bytes.forEach((byte) => { binary += String.fromCharCode(byte); });
  return btoa(binary).replace(/\+/g, '-').replace(/\//g, '_').replace(/=+$/g, '');
}

function fromBase64Url(value) {
  const base64 = value.replace(/-/g, '+').replace(/_/g, '/') + '='.repeat((4 - (value.length % 4)) % 4);
  const binary = atob(base64);
  return Uint8Array.from(binary, (character) => character.charCodeAt(0));
}

function prepareCreationOptions(options) {
  return {
    ...options,
    challenge: fromBase64Url(options.challenge),
    user: { ...options.user, id: fromBase64Url(options.user.id) },
    excludeCredentials: (options.excludeCredentials || []).map((item) => ({ ...item, id: fromBase64Url(item.id) })),
  };
}

function prepareRequestOptions(options) {
  return {
    ...options,
    challenge: fromBase64Url(options.challenge),
    allowCredentials: (options.allowCredentials || []).map((item) => ({ ...item, id: fromBase64Url(item.id) })),
  };
}

function credentialToJSON(credential) {
  const response = credential.response;
  const result = {
    id: credential.id,
    rawId: toBase64Url(credential.rawId),
    type: credential.type,
    response: {
      clientDataJSON: toBase64Url(response.clientDataJSON),
    },
    clientExtensionResults: credential.getClientExtensionResults(),
  };

  if (response.attestationObject) {
    result.response.attestationObject = toBase64Url(response.attestationObject);
    if (typeof response.getTransports === 'function') result.response.transports = response.getTransports();
  }
  if (response.authenticatorData) result.response.authenticatorData = toBase64Url(response.authenticatorData);
  if (response.signature) result.response.signature = toBase64Url(response.signature);
  if (response.userHandle) result.response.userHandle = toBase64Url(response.userHandle);
  return result;
}

async function apiRequest(path, body) {
  const response = await fetch(`${API_BASE}${path}`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    credentials: 'include',
    body: JSON.stringify(body),
  });
  const payload = await response.json().catch(() => ({}));
  if (!response.ok) throw new Error(payload.detail || payload.message || `Request failed (${response.status})`);
  return payload;
}

function ensureWebAuthnAvailable() {
  if (!window.isSecureContext) throw new Error('Passkeys require HTTPS, or localhost during development.');
  if (!window.PublicKeyCredential || !navigator.credentials) throw new Error('This browser does not support passkeys. Try a recent version of Safari, Chrome, or Edge.');
}

function setUnlocked(unlocked) {
  doorState.classList.toggle('is-unlocked', unlocked);
  doorState.innerHTML = `<span class="state-dot"></span> ${unlocked ? 'Unlock requested' : 'Locked'}`;
  lockPanel.hidden = !unlocked;
}

async function registerPasskey() {
  const username = usernameInput.value.trim();
  if (!username) throw new Error('Enter your account name before registering a passkey.');
  ensureWebAuthnAvailable();

  const options = await apiRequest('/webauthn/register/options', { username, door_id: doorId });
  const credential = await navigator.credentials.create({ publicKey: prepareCreationOptions(options) });
  if (!credential) throw new Error('Passkey registration was cancelled.');

  const result = await apiRequest('/webauthn/register/verify', {
    username,
    door_id: doorId,
    credential: credentialToJSON(credential),
  });
  setStatus(result.message || 'Passkey registered on this device. You can now authenticate.', 'success');
}

async function authenticate() {
  const username = usernameInput.value.trim();
  if (!username) throw new Error('Enter your account name to continue.');
  ensureWebAuthnAvailable();

  const options = await apiRequest('/webauthn/authenticate/options', { username, door_id: doorId });
  const credential = await navigator.credentials.get({ publicKey: prepareRequestOptions(options) });
  if (!credential) throw new Error('Authentication was cancelled.');

  const result = await apiRequest('/webauthn/authenticate/verify', {
    username,
    door_id: doorId,
    credential: credentialToJSON(credential),
  });

  if (result.status !== 'granted') throw new Error(result.message || 'Access was not granted.');
  setUnlocked(true);
  setStatus(result.message || 'Identity verified. The unlock request was sent to the door.', 'success');
}

async function lockDoor() {
  const result = await apiRequest('/access/lock', { door_id: doorId });
  setUnlocked(false);
  setStatus(result.message || 'Lock command sent to the door.', 'success');
}

async function runAction(button, action, defaultMessage) {
  setBusy(true, button);
  button.textContent = 'Please wait…';
  setStatus(defaultMessage);
  try {
    await action();
  } catch (error) {
    setStatus(error.message || 'Something went wrong. Please try again.', 'error');
  } finally {
    setBusy(false, button);
  }
}

registerButton.addEventListener('click', () => runAction(
  registerButton,
  registerPasskey,
  'Preparing secure passkey registration…',
));

authenticateButton.addEventListener('click', () => runAction(
  authenticateButton,
  authenticate,
  'Waiting for your device to verify your identity…',
));

lockButton.addEventListener('click', () => runAction(
  lockButton,
  lockDoor,
  'Sending lock command…',
));