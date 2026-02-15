#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

WebServer server(80);
WebSocketsServer webSocket(81);

const char *ssid = "wifigratis";
const char *password = "12345678";

String signalingPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 WebRTC</title>
</head>
<body>
  <h2>ESP32 WebRTC Demo</h2>
  <video id="localVideo" autoplay muted playsinline style="width:45%;"></video>
  <video id="remoteVideo" autoplay playsinline style="width:45%;"></video>
  <script>
    const ws = new WebSocket('ws://' + location.hostname + ':81/');
    let pc = new RTCPeerConnection();

    navigator.mediaDevices.getUserMedia({ video: true, audio: false })
      .then(stream => {
        document.getElementById('localVideo').srcObject = stream;
        stream.getTracks().forEach(track => pc.addTrack(track, stream));
      });

    pc.ontrack = event => {
      document.getElementById('remoteVideo').srcObject = event.streams[0];
    };

    ws.onmessage = async (event) => {
      let msg = JSON.parse(event.data);
      if (msg.sdp) {
        await pc.setRemoteDescription(new RTCSessionDescription(msg.sdp));
        if (msg.sdp.type === 'offer') {
          const answer = await pc.createAnswer();
          await pc.setLocalDescription(answer);
          ws.send(JSON.stringify({ sdp: pc.localDescription }));
        }
      } else if (msg.ice) {
        try { await pc.addIceCandidate(msg.ice); } catch(e) { console.error(e); }
      }
    };

    pc.onicecandidate = event => {
      if (event.candidate) {
        ws.send(JSON.stringify({ ice: event.candidate }));
      }
    };

    async function start() {
      const offer = await pc.createOffer();
      await pc.setLocalDescription(offer);
      ws.send(JSON.stringify({ sdp: pc.localDescription }));
    }
    start();
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", signalingPage);
}

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    webSocket.broadcastTXT(payload, length); // kirim ke semua client
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  Serial.println("AP Started: " + String(ssid));

  server.on("/", handleRoot);
  server.begin();

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();
}
