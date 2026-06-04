const state = {
  busy: false
};

const $ = (id) => document.getElementById(id);

function addMessage(role, text) {
  const node = document.createElement('article');
  node.className = `message ${role}`;
  const label = document.createElement('span');
  label.className = 'role';
  label.textContent = role === 'user' ? 'You' : role === 'assistant' ? 'BrainLLM' : 'System';
  const body = document.createElement('div');
  body.textContent = text;
  node.append(label, body);
  $('messages').appendChild(node);
  $('messages').scrollTop = $('messages').scrollHeight;
}

async function api(path, options = {}) {
  const response = await fetch(path, {
    headers: { 'Content-Type': 'application/json', ...(options.headers || {}) },
    ...options
  });
  const text = await response.text();
  let payload = {};
  try {
    payload = text ? JSON.parse(text) : {};
  } catch {
    payload = { error: text || 'Invalid JSON response' };
  }
  if (!response.ok || payload.error) {
    throw new Error(payload.error || `Request failed with status ${response.status}`);
  }
  return payload;
}

function setBusy(busy) {
  state.busy = busy;
  $('sendButton').disabled = busy;
  $('statusPill').textContent = busy ? 'Thinking' : 'Online';
  $('statusPill').className = `status-pill ${busy ? '' : 'ok'}`;
}

function renderStatus(status) {
  $('confidence').textContent = Number(status.confidence || 0).toFixed(2);
  $('accuracy').textContent = Number(status.accuracy || 0).toFixed(2);
  $('tokens').textContent = status.tokens_processed || 0;
  $('memoryUsage').textContent = Number(status.memory_usage || 0).toFixed(2);
  $('serverLine').textContent = 'Local API server is running on this host.';
  $('statusPill').textContent = 'Online';
  $('statusPill').className = 'status-pill ok';
}

async function refreshStatus() {
  try {
    renderStatus(await api('/api/status'));
  } catch (error) {
    $('serverLine').textContent = error.message;
    $('statusPill').textContent = 'Offline';
    $('statusPill').className = 'status-pill bad';
  }
}

async function loadAirLLM() {
  try {
    const config = await api('/api/airllm/config');
    $('modelId').value = config.model_id || '';
    $('pythonExecutable').value = config.python_executable || 'python';
    $('airTokens').value = config.max_new_tokens || 128;
    $('useCuda').checked = Boolean(config.use_cuda);
    $('airllmState').textContent = config.available
      ? 'AirLLM source and model are configured.'
      : 'AirLLM source is present only when paths and model id resolve.';
  } catch (error) {
    $('airllmState').textContent = error.message;
  }
}

$('chatForm').addEventListener('submit', async (event) => {
  event.preventDefault();
  if (state.busy) return;
  const prompt = $('promptInput').value.trim();
  if (!prompt) return;
  $('promptInput').value = '';
  addMessage('user', prompt);
  setBusy(true);
  try {
    const payload = await api('/api/chat', {
      method: 'POST',
      body: JSON.stringify({
        messages: [{ role: 'user', content: prompt }],
        max_tokens: Number($('maxTokens').value || 220)
      })
    });
    addMessage('assistant', payload.response || '');
    await refreshStatus();
  } catch (error) {
    addMessage('system', error.message);
  } finally {
    setBusy(false);
  }
});

$('resetButton').addEventListener('click', async () => {
  try {
    const payload = await api('/api/reset', { method: 'POST', body: '{}' });
    $('messages').innerHTML = '';
    addMessage('system', payload.message);
    await refreshStatus();
  } catch (error) {
    addMessage('system', error.message);
  }
});

$('airllmForm').addEventListener('submit', async (event) => {
  event.preventDefault();
  try {
    const payload = await api('/api/airllm/config', {
      method: 'POST',
      body: JSON.stringify({
        model_id: $('modelId').value.trim(),
        python_executable: $('pythonExecutable').value.trim() || 'python',
        max_new_tokens: Number($('airTokens').value || 128),
        use_cuda: $('useCuda').checked
      })
    });
    $('airllmState').textContent = `${payload.message} Available: ${payload.available ? 'yes' : 'no'}.`;
  } catch (error) {
    $('airllmState').textContent = error.message;
  }
});

$('memoryForm').addEventListener('submit', async (event) => {
  event.preventDefault();
  const query = encodeURIComponent($('memoryQuery').value.trim());
  try {
    const payload = await api(`/api/memory?query=${query}`);
    $('memoryResults').innerHTML = '';
    if (!payload.memories || payload.memories.length === 0) {
      $('memoryResults').textContent = 'No matching memory records yet.';
      return;
    }
    payload.memories.forEach((memory) => {
      const item = document.createElement('div');
      item.className = 'memory-item';
      item.textContent = `${memory.content} (${Number(memory.importance || 0).toFixed(2)})`;
      $('memoryResults').appendChild(item);
    });
  } catch (error) {
    $('memoryResults').textContent = error.message;
  }
});

addMessage('system', 'BrainLLM web client loaded. Use the chat box to talk to the local engine.');
refreshStatus();
loadAirLLM();
setInterval(refreshStatus, 5000);
