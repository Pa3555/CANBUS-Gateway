#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

// ============================================
// HTML WEB INTERFACE
// ============================================

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CANIMEX Gateway</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }

        .container {
            max-width: 1200px;
            margin: 0 auto;
        }

        .header {
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            margin-bottom: 20px;
            text-align: center;
        }

        .header h1 {
            color: #333;
            margin-bottom: 5px;
        }

        .header p {
            color: #666;
            font-size: 14px;
        }

        .status-badge {
            display: inline-block;
            padding: 5px 15px;
            border-radius: 20px;
            font-size: 12px;
            font-weight: bold;
            margin: 5px;
        }

        .status-online {
            background: #4CAF50;
            color: white;
        }

        .status-offline {
            background: #f44336;
            color: white;
        }

        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }

        .card {
            background: white;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }

        .card h2 {
            color: #333;
            margin-bottom: 15px;
            padding-bottom: 10px;
            border-bottom: 2px solid #667eea;
        }

        .info-row {
            display: flex;
            justify-content: space-between;
            padding: 10px 0;
            border-bottom: 1px solid #eee;
        }

        .info-label {
            color: #666;
            font-weight: 600;
        }

        .info-value {
            color: #333;
        }

        .btn {
            padding: 10px 20px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 14px;
            font-weight: 600;
            transition: all 0.3s;
            width: 100%;
            margin-top: 10px;
        }

        .btn-primary {
            background: #667eea;
            color: white;
        }

        .btn-primary:hover {
            background: #5568d3;
        }

        .btn-success {
            background: #4CAF50;
            color: white;
        }

        .btn-success:hover {
            background: #45a049;
        }

        .btn-danger {
            background: #f44336;
            color: white;
        }

        .btn-danger:hover {
            background: #da190b;
        }

        .form-group {
            margin-bottom: 15px;
        }

        .form-group label {
            display: block;
            margin-bottom: 5px;
            color: #333;
            font-weight: 600;
        }

        .form-group input,
        .form-group select {
            width: 100%;
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 5px;
            font-size: 14px;
        }

        .monitor {
            background: #1e1e1e;
            color: #00ff00;
            font-family: 'Courier New', monospace;
            padding: 15px;
            border-radius: 5px;
            height: 300px;
            overflow-y: auto;
            font-size: 12px;
            margin-top: 15px;
        }

        .monitor-line {
            margin-bottom: 5px;
            word-wrap: break-word;
        }

        .can-stats {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 10px;
            margin-top: 15px;
        }

        .stat-box {
            background: #f5f5f5;
            padding: 15px;
            border-radius: 5px;
            text-align: center;
        }

        .stat-value {
            font-size: 24px;
            font-weight: bold;
            color: #667eea;
        }

        .stat-label {
            font-size: 12px;
            color: #666;
            margin-top: 5px;
        }

        .toggle-switch {
            position: relative;
            display: inline-block;
            width: 50px;
            height: 24px;
        }

        .toggle-switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }

        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 24px;
        }

        .slider:before {
            position: absolute;
            content: "";
            height: 16px;
            width: 16px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }

        input:checked + .slider {
            background-color: #4CAF50;
        }

        input:checked + .slider:before {
            transform: translateX(26px);
        }

        @media (max-width: 768px) {
            .grid {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🚗 CANIMEX Gateway</h1>
            <p>ESP32-CAN-X2 Dual CAN Bus Interface</p>
            <div>
                <span class="status-badge" id="wifi-status">WiFi: Connecting...</span>
                <span class="status-badge" id="can1-status">CAN1: Unknown</span>
                <span class="status-badge" id="can2-status">CAN2: Unknown</span>
            </div>
        </div>

        <div class="grid">
            <!-- System Status -->
            <div class="card">
                <h2>📊 System Status</h2>
                <div class="info-row">
                    <span class="info-label">IP Address</span>
                    <span class="info-value" id="ip-addr">Loading...</span>
                </div>
                <div class="info-row">
                    <span class="info-label">WiFi Mode</span>
                    <span class="info-value" id="wifi-mode">Loading...</span>
                </div>
                <div class="info-row">
                    <span class="info-label">CAN1 Speed</span>
                    <span class="info-value" id="can1-speed">Loading...</span>
                </div>
                <div class="info-row">
                    <span class="info-label">CAN2 Speed</span>
                    <span class="info-value" id="can2-speed">Loading...</span>
                </div>
                <div class="info-row">
                    <span class="info-label">Uptime</span>
                    <span class="info-value" id="uptime">0s</span>
                </div>
            </div>

            <!-- System Resources -->
            <div class="card">
                <h2>🖥️ System Resources</h2>
                <div class="info-row">
                    <span class="info-label">Core 0 Load</span>
                    <span class="info-value" id="core0-load">0%</span>
                </div>
                <div class="info-row">
                    <span class="info-label">Core 1 Load</span>
                    <span class="info-value" id="core1-load">0%</span>
                </div>
                <div class="info-row">
                    <span class="info-label">Free Heap</span>
                    <span class="info-value" id="heap-free">0 KB</span>
                </div>
                <div class="info-row" id="psram-section" style="display:none;">
                    <span class="info-label">PSRAM Usage</span>
                    <span class="info-value" id="psram-usage">N/A</span>
                </div>
            </div>

            <!-- WiFi Configuration -->
            <div class="card">
                <h2>📡 WiFi Configuration</h2>
                <div class="form-group">
                    <label>Mode</label>
                    <select id="wifi-mode-select">
                        <option value="0">Access Point</option>
                        <option value="1">Client</option>
                    </select>
                </div>
                <div class="form-group">
                    <label>SSID</label>
                    <input type="text" id="ssid" placeholder="Enter SSID">
                </div>
                <div class="form-group">
                    <label>Password</label>
                    <input type="password" id="password" placeholder="Enter Password">
                </div>
                <button class="btn btn-primary" onclick="saveWiFi()">Save WiFi Config</button>
            </div>

            <!-- CAN Configuration -->
            <div class="card">
                <h2>🔧 CAN Configuration</h2>
                <div class="form-group">
                    <label>CAN1 Speed</label>
                    <select id="can1-speed-select">
                        <option value="0">125 kbps</option>
                        <option value="1">250 kbps</option>
                        <option value="2">500 kbps</option>
                        <option value="3">1000 kbps</option>
                    </select>
                </div>
                <div class="form-group">
                    <label>CAN2 Speed</label>
                    <select id="can2-speed-select">
                        <option value="0">125 kbps</option>
                        <option value="1">250 kbps</option>
                        <option value="2">500 kbps</option>
                        <option value="3">1000 kbps</option>
                    </select>
                </div>
                <button class="btn btn-primary" onclick="saveCANConfig()">Save CAN Config</button>
                <button class="btn btn-danger" onclick="resetDefaults()">Reset to Defaults</button>
            </div>
        </div>

        <!-- CAN Monitor -->
        <div class="card">
            <h2>📡 CAN Bus Monitor</h2>
            <div style="display: flex; justify-content: space-between; align-items: center;">
                <div>
                    <label class="toggle-switch">
                        <input type="checkbox" id="monitor-toggle" onchange="toggleMonitor()">
                        <span class="slider"></span>
                    </label>
                    <span style="margin-left: 10px; font-weight: 600;">Real-time Monitoring</span>
                </div>
                <button class="btn btn-danger" onclick="clearMonitor()" style="width: auto; margin: 0;">Clear</button>
            </div>

            <div class="can-stats">
                <div class="stat-box">
                    <div class="stat-value" id="can1-count">0</div>
                    <div class="stat-label">CAN1 Messages</div>
                </div>
                <div class="stat-box">
                    <div class="stat-value" id="can2-count">0</div>
                    <div class="stat-label">CAN2 Messages</div>
                </div>
            </div>

            <div class="monitor" id="can-monitor">
                <div class="monitor-line">📡 CAN Bus Monitor Ready...</div>
                <div class="monitor-line">Toggle switch above to start monitoring</div>
            </div>
        </div>
    </div>

    <script>
        let ws;
        let monitorEnabled = false;

        function connectWebSocket() {
            ws = new WebSocket('ws://' + window.location.hostname + '/ws');

            ws.onopen = function() {
                console.log('WebSocket connected');
                getStatus();
            };

            ws.onmessage = function(event) {
                handleMessage(JSON.parse(event.data));
            };

            ws.onclose = function() {
                console.log('WebSocket disconnected, reconnecting...');
                setTimeout(connectWebSocket, 2000);
            };
        }

        function handleMessage(data) {
            if (data.type === 'status') {
                updateStatus(data);
            } else if (data.type === 'can') {
                addCANMessage(data);
            }
        }

        function updateStatus(data) {
            document.getElementById('ip-addr').textContent = data.ip;
            document.getElementById('wifi-mode').textContent = data.wifi_mode;
            document.getElementById('can1-speed').textContent = data.can1_speed;
            document.getElementById('can2-speed').textContent = data.can2_speed;
            document.getElementById('uptime').textContent = data.uptime;
            document.getElementById('can1-count').textContent = data.can1_count;
            document.getElementById('can2-count').textContent = data.can2_count;

            // Update status badges
            document.getElementById('wifi-status').textContent = 'WiFi: ' + data.wifi_mode;
            document.getElementById('wifi-status').className = 'status-badge status-online';

            document.getElementById('can1-status').textContent = 'CAN1: ' + (data.can1_running ? 'Running' : 'Stopped');
            document.getElementById('can1-status').className = 'status-badge ' + (data.can1_running ? 'status-online' : 'status-offline');

            document.getElementById('can2-status').textContent = 'CAN2: ' + (data.can2_running ? 'Running' : 'Stopped');
            document.getElementById('can2-status').className = 'status-badge ' + (data.can2_running ? 'status-online' : 'status-offline');

            // Update system resources
            document.getElementById('core0-load').textContent = data.core0_load.toFixed(1) + '%';
            document.getElementById('core1-load').textContent = data.core1_load.toFixed(1) + '%';
            document.getElementById('heap-free').textContent = (data.heap_free / 1024).toFixed(1) + ' KB';

            // Update PSRAM if available
            if (data.psram_available) {
                document.getElementById('psram-section').style.display = 'flex';
                const psramUsedMB = (data.psram_used / 1048576).toFixed(2);
                const psramTotalMB = (data.psram_total / 1048576).toFixed(2);
                const psramPct = data.psram_usage_pct.toFixed(1);
                document.getElementById('psram-usage').textContent = psramUsedMB + ' / ' + psramTotalMB + ' MB (' + psramPct + '%)';
            } else {
                document.getElementById('psram-section').style.display = 'none';
            }

            // Update form values
            document.getElementById('wifi-mode-select').value = data.wifi_mode_val;
            document.getElementById('can1-speed-select').value = data.can1_speed_val;
            document.getElementById('can2-speed-select').value = data.can2_speed_val;
        }

        function addCANMessage(data) {
            const monitor = document.getElementById('can-monitor');
            const line = document.createElement('div');
            line.className = 'monitor-line';

            const timestamp = new Date().toLocaleTimeString();
            line.textContent = `[${timestamp}] [CAN${data.channel}] ID:0x${data.id} [${data.dlc}] ${data.data}`;

            monitor.appendChild(line);

            // Keep only last 100 messages
            while (monitor.children.length > 100) {
                monitor.removeChild(monitor.firstChild);
            }

            monitor.scrollTop = monitor.scrollHeight;
        }

        function getStatus() {
            fetch('/api/status')
                .then(r => r.json())
                .then(data => updateStatus(data))
                .catch(e => console.error('Status fetch error:', e));
        }

        function saveWiFi() {
            const mode = document.getElementById('wifi-mode-select').value;
            const ssid = document.getElementById('ssid').value;
            const password = document.getElementById('password').value;

            fetch('/api/wifi', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({mode, ssid, password})
            })
            .then(r => r.json())
            .then(data => {
                alert(data.message);
                if (data.success) {
                    document.getElementById('ssid').value = '';
                    document.getElementById('password').value = '';
                }
            })
            .catch(e => alert('Error saving WiFi config'));
        }

        function saveCANConfig() {
            const can1_speed = document.getElementById('can1-speed-select').value;
            const can2_speed = document.getElementById('can2-speed-select').value;

            fetch('/api/can', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({can1_speed, can2_speed})
            })
            .then(r => r.json())
            .then(data => alert(data.message))
            .catch(e => alert('Error saving CAN config'));
        }

        function toggleMonitor() {
            monitorEnabled = document.getElementById('monitor-toggle').checked;

            fetch('/api/monitor', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({enabled: monitorEnabled})
            });

            if (monitorEnabled) {
                clearMonitor();
                addSystemMessage('🟢 Monitoring started');
            } else {
                addSystemMessage('🔴 Monitoring stopped');
            }
        }

        function clearMonitor() {
            document.getElementById('can-monitor').innerHTML = '';
        }

        function addSystemMessage(msg) {
            const monitor = document.getElementById('can-monitor');
            const line = document.createElement('div');
            line.className = 'monitor-line';
            line.style.color = '#ffff00';
            line.textContent = msg;
            monitor.appendChild(line);
        }

        function resetDefaults() {
            if (confirm('Reset all settings to defaults? Device will restart.')) {
                fetch('/api/reset', {method: 'POST'})
                    .then(() => alert('Device resetting...'))
                    .catch(e => console.error('Reset error:', e));
            }
        }

        // Initialize
        connectWebSocket();
        setInterval(getStatus, 2000);  // Update status every 2 seconds
    </script>
</body>
</html>
)rawliteral";

#endif // WEB_INTERFACE_H
