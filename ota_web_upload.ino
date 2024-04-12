#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

const char* host = "esp32";
const char* ssid = "Home";
const char* password = "9503297281";

WebServer server(80);

/*
 * Server Index Page
 */
String serverIndex = 
  "<form method='POST' action='/update' enctype='multipart/form-data'>"
    "<input type='file' name='update'>"
    "<input type='submit' value='Update'>"
  "</form>"
  "<div id='result'></div>"
  "<script>"
    "window.onload = function() {"
      "var urlParams = new URLSearchParams(window.location.search);"
      "var result = urlParams.get('result');"
      "if (result) {"
        "document.getElementById('result').innerText = result;"
      "}"
    "};"
  "</script>";

/*
 * setup function
 */
void setup(void) {
  Serial.begin(115200);
  //  pinMode(4, OUTPUT);
	
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize mDNS
  if (!MDNS.begin(host)) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  // Set up route for serving the server index page
  server.on("/serverIndex", HTTP_GET, []() {
    server.send(200, "text/html", serverIndex);
  });

  // Handling firmware update
  server.on("/update", HTTP_POST, []() {
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        // Redirect to serverIndex page with update status
        server.sendHeader("Location", "/serverIndex?result=OK");
        server.send(303);
      } else {
        Update.printError(Serial);
        // Redirect to serverIndex page with update status
        server.sendHeader("Location", "/serverIndex?result=FAIL");
        server.send(303);
      }
    }
  });

  server.begin();
}

void loop(void) {
  server.handleClient();

  // digitalWrite(4, HIGH); // turn the LED on
  // delay(500);             // wait for 500 milliseconds
  // digitalWrite(4, LOW);  // turn the LED off
  // delay(500);             // wait for 500 milliseconds
}
