#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AMG88xx.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Servo.h>

#define DHTPIN 2            // Pin where the DHT sensor is connected
#define DHTTYPE DHT22       // DHT sensor type (DHT22)

#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"

#define CAM_SERIAL Serial1  // Camera module serial port
#define CAM_SHUTTER 10       // Camera shutter control pin

#define SERVO_PIN 11         // Pin for controlling the servo (heater)

#define IMAGE_INTERVAL 600000 // Image capture interval in milliseconds (10 minutes)

DHT dht(DHTPIN, DHTTYPE);

Adafruit_AMG88xx amg;

Servo heater;

unsigned long previousImageTime = 0;

void setup() {
  Serial.begin(9600);
  CAM_SERIAL.begin(38400);
  heater.attach(SERVO_PIN);

  // Connect to WiFi
  connectToWiFi();

  // Initialize DHT sensor
  dht.begin();

  // Initialize AMG88xx sensor
  if (!amg.begin()) {
    Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
    while (1);
  }
}

void loop() {
  // Read temperature and humidity
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Control the heater based on temperature
  controlHeater(temperature);

  // Capture images, videos, and audio recordings
  captureData();

  // Delay between iterations
  delay(1000);
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void controlHeater(float temperature) {
  // Adjust the heater based on the temperature
  if (temperature < 25.0) {
    heater.write(180);  // Turn on the heater
  } else {
    heater.write(0);    // Turn off the heater
  }
}

void captureData() {
  unsigned long currentMillis = millis();

  // Capture images at a regular interval
  if (currentMillis - previousImageTime >= IMAGE_INTERVAL) {
    captureImage();
    previousImageTime = currentMillis;
  }

  // Capture other data (videos, audio recordings) as needed
  // Implement your code for capturing videos and audio recordings here
}

void captureImage() {
  // Send command to the camera module to capture an image
  CAM_SERIAL.println("CAPTURE");

  // Wait for the camera to capture the image
  delay(2000);

  // Send captured image to a server using HTTP POST
  sendImageToServer();
}

void sendImageToServer() {
  HTTPClient http;

  // Replace SERVER_IP and SERVER_PORT with the server's IP address and port
  String serverAddress = "http://SERVER_IP:SERVER_PORT/upload";
  
  // Create a unique file name for each image (you may need to implement this logic)
  String fileName = "image.jpg";

  // Open the image file
  File imageFile = SPIFFS.open(fileName, "r");

  if (http.begin(serverAddress)) {
    // Set the Content-Type header to multipart/form-data
    http.addHeader("Content-Type", "multipart/form-data");

    // Use the POST method
    int httpCode = http.POST(imageFile, fileName);

    // Check the result
    if (httpCode > 0) {
      Serial.printf("HTTP POST Success, Code: %d\n", httpCode);
    } else {
      Serial.printf("HTTP POST Failed, Error: %s\n", http.errorToString(httpCode).c_str());
    }

    // Close the connection
    http.end();
  } else {
    Serial.println("Unable to connect to server");
  }

  // Close the image file
  imageFile.close();
}
