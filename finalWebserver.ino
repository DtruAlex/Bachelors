#include <WiFi.h>
#include <WebServer.h>
#include <time.h>
#include "Wire.h" // Include the Wire library for I2C communication

#define I2C_DEV_ADDR 0x55 // Define the I2C slave device address

// Define GPIO pins for the LEDs
const int LED_PIN_1 = 2;  // Example GPIO pin for LED 1 (often corresponds to onboard LED)
const int LED_PIN_2 = 4;  // Example GPIO pin for LED 2, choose an available pin

// --- Struct Definition for Application Data ---
struct AppData {
  int ledState[2] = {LOW, LOW}; // Array to store states of LED 1 and LED 2 (HIGH or LOW)
  String stationIP = "N/A";     // String to store the Station Mode IP address
};

// --- Create a global instance of the struct ---
AppData myData;

// Station Mode Credentials (These can be defaults or last connected)
// Using String objects for easier modification from web form input.
String sta_ssid = ""; // You might want to save/load this from EEPROM/SPIFFS in a real application
String sta_password = ""; // You might want to save/load this securely

// AP Mode Credentials
const char* ap_ssid = "Smarthome Controller";
const char* ap_password = "12345678"; // Choose a strong password

// IP Configurations for AP
IPAddress local_ip(192, 168, 4, 1); // Changed to a less common subnet to avoid conflicts
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0; // Adjust for your timezone (e.g., 3600 for +1, 7200 for +2)
const int daylightOffset_sec = 3600; // Adjust for daylight saving (0 or 3600 typically)

bool is_authenticated() {
  // Simple HTTP Basic Authentication
  // In a production environment, use more secure authentication methods
  if (!server.authenticate("admin", "admin")) {
    server.requestAuthentication();
    return false;
  }
  return true;
}

// Helper function to generate the standard page structure
String getPage(String content) {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>"; // Added charset
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>SmartHome Controller</title>";
  html += "<style>";
  html += "body{font-family:Arial,sans-serif;margin:0;padding:20px;background:#f0f2f5;}";
  html += "nav{background:#333;padding:10px;margin-bottom:20px;text-align:center;}"; // Centered nav
  html += "nav a{color:white;text-decoration:none;margin:0 10px;}"; // Adjusted margin
  html += "nav a:hover{text-decoration:underline;}"; // Added hover effect
  html += ".container{max-width:800px;margin:0 auto;background:white;padding:20px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);}";
  html += "h1, h2{color:#333;}"; // Added heading color
  html += "form{margin-top:20px;}";
  html += "label{display:block;margin-bottom:5px;font-weight:bold;}";
  html += "input[type='text'], input[type='password']{width:calc(100% - 22px);padding:10px;margin-bottom:10px;border:1px solid #ccc;border-radius:4px;}";
  html += "input[type='submit']{background-color:#4CAF50;color:white;padding:10px 20px;border:none;border-radius:4px;cursor:pointer;font-size:16px;}";
  html += "input[type='submit']:hover{background-color:#45a049;}";
  html += ".message{margin-top:15px;padding:10px;border-radius:4px;}";
  html += ".success{background-color:#d4edda;color:#155724;border-color:#c3e6cb;border-style: solid;border-width: 1px;}"; // Added border style
  html += ".error{background-color:#f8d7da;color:#721c24;border-color:#f5c6cb;border-style: solid;border-width: 1px;}"; // Added border style
  html += ".info{background-color:#e3f2fd;color:#01579b;border-color:#bbdefb;border-style: solid;border-width: 1px;}"; // Added info box style
  html += ".warning{background-color:#fff9c4;color:#fbc02d;border-color:#fff59d;border-style: solid;border-width: 1px;}"; // Added warning box style
  html += ".led-control{margin-bottom:15px;padding:15px;border:1px solid #ddd;border-radius:5px;display: flex; justify-content: space-between; align-items: center;}"; // Flexbox for layout
  html += ".led-control div { flex-grow: 1; }"; // Allow text part to grow
  html += ".led-control p { margin: 5px 0; }"; // Reduce paragraph margin
  html += ".led-control button{padding:8px 15px;margin-left:10px;border:none;border-radius:4px;cursor:pointer; flex-shrink: 0;}"; // Added flex-shrink to buttons
  html += ".led-on{background-color:#4CAF50;color:white;}";
  html += ".led-on:hover{background-color:#45a049;}"; // Added hover
  html += ".led-off{background-color:#f44336;color:white;}";
  html += ".led-off:hover{background-color:#d32f2f;}"; // Added hover
  html += "@keyframes fadeIn{from{opacity:0;}to{opacity:1;}}";
  html += ".animate{animation:fadeIn 0.5s ease-in;}"; // Adjusted animation duration and timing
  html += "</style></head><body>";
  html += "<nav>";
  html += "<a href='/'>Status</a>";
  html += "<a href='/info'>Connection Info</a>";
  html += "<a href='/admin'>Admin</a>";
  html += "<a href='/LED'>LED Control</a>";
  html += "</nav>";
  html += "<div class='container animate'>";
  html += content;
  html += "</div></body></html>";
  return html;
}

void handleHome() {
  String content = "<h1>Welcome to your SmartHome</h1>";
  content += "<div style='text-align:center;'>";
  content += "<h2>Current Time</h2>";
  content += "<div id='clock' style='font-size:48px;text-align:center;'>Loading...</div>";
  // JavaScript for updating clock dynamically
  content += "<script>";
  content += "function updateClock(){";
  content += "const now=new Date();";
  // Format time with leading zeros
  content += "const hours = String(now.getHours()).padStart(2, '0');";
  content += "const minutes = String(now.getMinutes()).padStart(2, '0');";
  content += "const seconds = String(now.getSeconds()).padStart(2, '0');";
  content += "document.getElementById('clock').textContent= `${hours}:${minutes}:${seconds}`;";
  content += "}";
  content += "setInterval(updateClock,1000);updateClock();";
  content += "</script>";

  content += "<p>Welcome! Explore our pages using the navigation bar above.</p>";

  // Display LED states on the status page using myData
  content += "<div class='warning' style='margin-top:20px;'>"; // Using warning class for status display
  content += "<h2>LED Status</h2>";
  content += "<p><strong>LED 1:</strong> ";
  content += (myData.ledState[0] == HIGH) ? "<span style='color:green;font-weight:bold;'>ON</span>" : "<span style='color:red;font-weight:bold;'>OFF</span>";
  content += "</p>";
  content += "<p><strong>LED 2:</strong> ";
  content += (myData.ledState[1] == HIGH) ? "<span style='color:green;font-weight:bold;'>ON</span>" : "<span style='color:red;font-weight:bold;'>OFF</span>";
  content += "</p>";
  content += "</div>";


  content += "<div class='info' style='margin-top:20px;'>"; // Using info class for network info
  content += "<h2>Network Information</h2>";
  content += "<p><strong>WiFi Station IP:</strong> ";
  content += myData.stationIP; // Display IP from myData
  content += "</p>";
  content += "<p><strong>Access Point IP:</strong> ";
  content += WiFi.softAPIP().toString();
  content += "</p>";
  content += "<p><strong>Access Point SSID:</strong> ";
  content += ap_ssid;
  content += "</p>";
  content += "</div>";
  content += "</div>";
  server.send(200, "text/html", getPage(content));
}

void handleInfo() {
  String content = "<h1>Detailed Network Information</h1>";
  content += "<div class='info'>"; // Using info class
  content += "<h2>Station Mode (Client)</h2>";
  content += "<p><strong>Status:</strong> ";
  content += (WiFi.status() == WL_CONNECTED) ? "Connected" : "Disconnected";
  content += "</p>";
  content += "<p><strong>IP Address:</strong> ";
  // Update and display IP from myData
  if (WiFi.status() == WL_CONNECTED) {
     myData.stationIP = WiFi.localIP().toString();
  } else {
     myData.stationIP = "N/A";
  }
  content += myData.stationIP;
  content += "</p>";
  content += "<p><strong>Subnet Mask:</strong> ";
  content += (WiFi.status() == WL_CONNECTED) ? WiFi.subnetMask().toString() : "N/A";
  content += "</p>";
  content += "<p><strong>Gateway IP:</strong> ";
  content += (WiFi.status() == WL_CONNECTED) ? WiFi.gatewayIP().toString() : "N/A";
  content += "</p>";
  content += "<p><strong>MAC Address:</strong> ";
  content += WiFi.macAddress();
  content += "</p>";
  content += "<p><strong>Configured SSID:</strong> "; // Changed text to reflect configured SSID
  content += sta_ssid; // Use global sta_ssid
  content += "</p>";
  content += "</div>";

  content += "<div class='info' style='margin-top:20px;'>"; // Added margin
  content += "<h2>Access Point Mode</h2>";
  content += "<p><strong>AP IP:</strong> ";
  content += WiFi.softAPIP().toString();
  content += "</p>";
  content += "<p><strong>AP MAC Address:</strong> ";
  content += WiFi.softAPmacAddress();
  content += "</p>";
  content += "<p><strong>AP SSID:</strong> ";
  content += ap_ssid;
  content += "</p>";
  content += "<p><strong>Connected Stations:</strong> ";
  content += WiFi.softAPgetStationNum();
  content += "</p>";
  content += "</div>";
  server.send(200, "text/html", getPage(content));
}

void handleAdmin() {
  if (!is_authenticated()) {
    return; // Authentication failed, server.requestAuthentication() was called
  }
  String content = "<h1>Admin Panel - WiFi Configuration</h1>";
  content += "<p>Configure the WiFi network you want the ESP32 to connect to (Station Mode).</p>";
  content += "<form method='POST' action='/connect_wifi'>";
  content += "<label for='ssid'>WiFi SSID:</label>";
  content += "<input type='text' id='ssid' name='ssid' required value='" + sta_ssid + "'><br><br>"; // Pre-fill with current SSID
  content += "<label for='password'>WiFi Password:</label>";
  content += "<input type='password' id='password' name='password' value='" + sta_password + "'><br><br>"; // Pre-fill with current password (consider security implications)
  content += "<input type='submit' value='Connect to WiFi'>";
  content += "</form>";

  server.send(200, "text/html", getPage(content));
}

// --- Function to send data over I2C (Sends LED states and IP) ---
void sendDataOverI2C(const AppData& dataToSend) {
  Serial.println("Sending data over I2C...");
  Wire.beginTransmission(I2C_DEV_ADDR); // Begin transmission to I2C slave device

  // Send the LED states (2 bytes)
  Wire.write((byte)dataToSend.ledState[0]); // Send state of LED 1
  Wire.write((byte)dataToSend.ledState[1]); // Send state of LED 2

  // Send the IP address (fixed 16 bytes for IPv4 + null)
  const int ipBufferSize = 16;
  char ipBuffer[ipBufferSize]; // No need for +1 here, Wire.write handles null if present
  // Copy the string into the buffer, ensuring it's null-padded if shorter
  dataToSend.stationIP.toCharArray(ipBuffer, sizeof(ipBuffer));

  Wire.write((byte*)ipBuffer, sizeof(ipBuffer)); // Send the IP address buffer

  byte error = Wire.endTransmission(); // End transmission and check for errors

  if (error == 0) {
    Serial.println("I2C data (LED states + IP) sent successfully.");
  } else {
    Serial.print("I2C transmission error sending data: ");
    Serial.println(error); // 1: data too long, 2: received NACK on transmit of address, 3: received NACK on transmit of data, 4: other error
  }
}
// --- End I2C Send Function ---


void handleConnectWifi() {
  if (!is_authenticated()) {
    return; // Authentication failed
  }

  String content = "<h1>WiFi Connection Status</h1>";

  if (server.method() == HTTP_POST) {
    if (server.hasArg("ssid")) {
      String new_ssid = server.arg("ssid");
      String new_password = server.arg("password");

      // Update the global String objects
      sta_ssid = new_ssid;
      sta_password = new_password;

      content += "<p>Attempting to connect to SSID: <strong>" + sta_ssid + "</strong></p>";

      // Send an immediate response indicating the attempt
      server.send(200, "text/html", getPage(content + "<p>Please wait while the ESP32 connects...</p>"));

      // Allow the response to be sent before blocking with WiFi.begin
      delay(100);
      Serial.println("\nConnecting to new WiFi network...");
      Serial.print("SSID: ");
      Serial.println(sta_ssid);

      // Disconnect from current network if any
      WiFi.disconnect(true);
      delay(500);

      // Attempt to connect to the new network
      // Use c_str() to get const char* from String for WiFi.begin
      WiFi.begin(sta_ssid.c_str(), sta_password.c_str());

      int attempts = 0;
      int max_attempts = 40; // Increased attempts (e.g., 20 seconds timeout)

      while (WiFi.status() != WL_CONNECTED && attempts < max_attempts) {
        delay(500);
        Serial.print(".");
        attempts++;
      }
      Serial.println(); // New line after dots

      // After the connection attempt (which happens in the background/yield)
      // We need to refresh the page or redirect to show the result.
      // A simple redirect back to the admin page or status page is common.
      // For this example, let's redirect to the status page to see the IP.
      // You might add a status message parameter to the redirect URL for better feedback.

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Successfully connected to WiFi!");
        Serial.print("Station IP address: ");
        Serial.println(WiFi.localIP());

        // Update the IP in the struct
        myData.stationIP = WiFi.localIP().toString();

        // --- Send the combined data over I2C after successful connection ---
        Serial.println("WiFi connected, sending data over I2C...");
        sendDataOverI2C(myData);
        // --- End Send Data ---


        // Set time after successful connection
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
         Serial.println("Time configured via NTP.");

        // Redirect to status page with success message (simplified)
        server.sendHeader("Location", "/?msg=connect_success"); // Add a message parameter
        server.send(303); // Use 303 See Other
      } else {
        Serial.println("Failed to connect to WiFi network.");
         // Set IP in struct to N/A on failure
        myData.stationIP = "N/A";
        // --- Send the combined data over I2C after connection failure ---
        Serial.println("WiFi failed to connect, sending data over I2C...");
        sendDataOverI2C(myData);
        // --- End Send Data ---

        // Redirect to admin page with error message (simplified)
        server.sendHeader("Location", "/admin?msg=connect_fail"); // Add a message parameter
        server.send(303); // Use 303 See Other
      }
       // IMPORTANT: The above redirects send the response and end the function.
       // The following code will only run if there was an error *before*
       // attempting WiFi.begin (e.g., missing SSID).
    } else {
      content += "<div class='message error'><p>Error: SSID parameter missing.</p></div>";
      server.send(200, "text/html", getPage(content + "<p><a href='/admin'>Back to Admin Page</a></p>"));
    }
  } else {
    content += "<div class='message error'><p>Error: Invalid request method. Please use the form on the admin page.</p></div>";
     server.send(200, "text/html", getPage(content + "<p><a href='/admin'>Back to Admin Page</a></p>"));
  }

  // Note: The code below will likely not be reached if a redirect occurred above.
  // Consider removing or restructuring if not needed.
}


// Handler for the /LED page
void handleLED() {
  if (!is_authenticated()) {
    return;
  }

  String content = "<h1>LED Control</h1>";

  // LED 1 Control
  content += "<div class='led-control'>";
  content += "<div><h2>LED 1</h2>"; // Put heading and status in a div
  content += "<p>Current State: ";
  // Display state from myData
  content += (myData.ledState[0] == HIGH) ? "<span style='color:green;font-weight:bold;'>ON</span>" : "<span style='color:red;font-weight:bold;'>OFF</span>";
  content += "</p></div>";
  content += "<div>"; // Put buttons in a div for flexbox
  // Links use query parameters
  content += "<a href='/led_control?led=1&state=on'><button class='led-on'>Turn ON</button></a>";
  content += "<a href='/led_control?led=1&state=off'><button class='led-off'>Turn OFF</button></a>";
  content += "</div>";
  content += "</div>";

  // LED 2 Control
  content += "<div class='led-control'>";
  content += "<div><h2>LED 2</h2>"; // Put heading and status in a div
  content += "<p>Current State: ";
  // Display state from myData
  content += (myData.ledState[1] == HIGH) ? "<span style='color:green;font-weight:bold;'>ON</span>" : "<span style='color:red;font-weight:bold;'>OFF</span>";
  content += "</p></div>";
  content += "<div>"; // Put buttons in a div for flexbox
  // Links use query parameters
  content += "<a href='/led_control?led=2&state=on'><button class='led-on'>Turn ON</button></a>";
  content += "<a href='/led_control?led=2&state=off'><button class='led-off'>Turn OFF</button></a>";
  content += "</div>";
  content += "</div>";

  server.send(200, "text/html", getPage(content));
}

// Combined handler for controlling LEDs
void handleLEDControl() {
  if (!is_authenticated()) {
    return; // Authentication failed
  }

  String ledNumberStr = "";
  String stateStr = "";

  // Read from query parameters
  if (server.hasArg("led")) {
      ledNumberStr = server.arg("led");
  } else {
      server.send(400, "text/plain", "Missing 'led' parameter.");
      return;
  }

  if (server.hasArg("state")) {
      stateStr = server.arg("state");
  } else {
      server.send(400, "text/plain", "Missing 'state' parameter.");
      return;
  }

  Serial.print("Received LED control request: LED=");
  Serial.print(ledNumberStr);
  Serial.print(", State=");
  Serial.println(stateStr);

  int ledNumber = ledNumberStr.toInt();    // Convert LED number string to integer
  int newState = LOW;                      // Default state is LOW

  // Determine the desired state (ON or OFF)
  if (stateStr.equalsIgnoreCase("on")) {
    newState = HIGH;
  } else if (stateStr.equalsIgnoreCase("off")) {
    newState = LOW;
  } else {
    // Handle invalid state parameter
    server.send(400, "text/plain", "Invalid state parameter. Use 'on' or 'off'.");
    return;
  }

  // Control the specified LED and update state in the struct
  if (ledNumber == 1) {
    digitalWrite(LED_PIN_1, newState);
    myData.ledState[0] = newState;  // Update the state variable in the struct
    Serial.println("Controlled LED 1");
  } else if (ledNumber == 2) {
    digitalWrite(LED_PIN_2, newState);
    myData.ledState[1] = newState;  // Update the state variable in the struct
     Serial.println("Controlled LED 2");
  } else {
    // Handle invalid LED number
    server.send(400, "text/plain", "Invalid LED number. Use 1 or 2.");
    return;
  }

  // --- Call the I2C send function after updating the state ---
  // This will now send both LED states and the IP address
  sendDataOverI2C(myData);
  // --- End Call ---

  // Redirect back to the LED control page to show the updated state
  // Using 303 See Other is appropriate for redirecting after a POST-like action (GET request with side effects)
  server.sendHeader("Location", "/LED");
  server.send(303);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  String content = "<h1>404: Not Found</h1><p>The requested page was not found.</p>";
  content += "<p>Details:</p><pre>" + message + "</pre>"; // Display details in preformatted text

  server.send(404, "text/html", getPage(content));
  Serial.println("404 Not Found: " + server.uri()); // Log 404s to serial
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Longer delay to allow serial monitor to open reliably

  // --- Initialize I2C (Wire) ---
  // Default pins for ESP32 are SDA=21, SCL=22. Adjust if needed.
  Wire.begin();
  Serial.println("I2C (Wire) initialized.");
  // --- End I2C Initialization ---


  // Initialize LED pins as outputs
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  // Set initial state to LOW and update struct
  digitalWrite(LED_PIN_1, LOW);
  digitalWrite(LED_PIN_2, LOW);
  myData.ledState[0] = LOW; // Ensure struct variable matches initial state
  myData.ledState[1] = LOW;


  // Set WiFi mode to AP_STA
  WiFi.mode(WIFI_AP_STA);

  // Configure and start Access Point
  WiFi.softAP(ap_ssid, ap_password);
  WiFi.softAPConfig(local_ip, gateway, subnet);

  Serial.println("Setting up Access Point: ");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("AP SSID: ");
  Serial.println(ap_ssid);


  // Attempt to connect to the default or previously saved Station Mode network
  // In this simplified example, sta_ssid and sta_password are empty strings initially,
  // so this attempt will likely fail unless you hardcode defaults or load them.
  Serial.println("\nAttempting to connect to default WiFi network (if configured)...");
  Serial.print("SSID: '");
  Serial.print(sta_ssid);
  Serial.println("'");

  if (sta_ssid.length() > 0) { // Only try connecting if an SSID is set
    WiFi.begin(sta_ssid.c_str(), sta_password.c_str());

    int attempts = 0;
    int max_attempts = 30; // Try for 15 seconds

    while (WiFi.status() != WL_CONNECTED && attempts < max_attempts) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
     Serial.println(); // New line after dots

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to WiFi Station Mode");
      Serial.print("Station IP address: ");
      Serial.println(WiFi.localIP());
      // Update the IP in the struct upon successful connection
      myData.stationIP = WiFi.localIP().toString();

      // --- Send the initial combined data over I2C after successful connection ---
      Serial.println("WiFi connected in setup, sending data over I2C...");
      sendDataOverI2C(myData);
      // --- End Send Data ---

       // Set time after successful connection
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
       Serial.println("Time configured via NTP.");
    } else {
      Serial.println("Failed to connect to default WiFi network.");
      // Set IP in struct to N/A on failure
      myData.stationIP = "N/A";
      // --- Send the initial combined data over I2C after connection failure ---
      Serial.println("WiFi failed to connect in setup, sending data over I2C...");
      sendDataOverI2C(myData);
      // --- End Send Data ---
      Serial.println("Connect to the AP (" + String(ap_ssid) + ") and use the Admin page to configure Station mode.");
    }
  } else {
       Serial.println("No default Station SSID configured. AP mode is active.");
        // Set IP in struct to N/A if no SSID configured
       myData.stationIP = "N/A";
       // --- Send the initial combined data over I2C if no SSID configured ---
       Serial.println("No default SSID, sending initial data over I2C...");
       sendDataOverI2C(myData);
       // --- End Send Data ---
  }


  // Server Handlers
  server.on("/", handleHome);
  server.on("/admin", handleAdmin);
  server.on("/connect_wifi", HTTP_POST, handleConnectWifi);
  server.on("/info", handleInfo);
  server.on("/LED", handleLED); // Handler for the LED control page

  // Register handleLEDControl for the base path /led_control
  server.on("/led_control", handleLEDControl);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started on port 80");
}

void loop() {
  server.handleClient();
   // Add a small delay to avoid watchdog timer resets, especially if loop is very fast
  delay(5); // Recommended small delay in loop

  // --- Accessing data from the struct for serial print ---
  // These lines are just for debugging and can be removed or placed inside a conditional
  // if you only want to print when something changes or periodically.
  // Serial.print("LED 1 State: ");
  // Serial.println(myData.ledState[0]);
  // Serial.print("LED 2 State: ");
  // Serial.println(myData.ledState[1]);
  // Serial.print("Station IP: ");
  // Serial.println(myData.stationIP);
  // delay(1000); // Add a delay if printing frequently to avoid flooding serial
  // --- End Debugging Print ---
}
