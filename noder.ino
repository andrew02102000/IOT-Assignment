
/** Application Initialisation Function****/
void setup() {

  dht.setup(D4, DHTesp::DHT11); //Set up DHT11 sensor
  pinMode(led, OUTPUT); //set up LED
  Serial.begin(9600);
  while (!Serial) delay(1);
  setup_wifi();

  #ifdef ESP8266
    espClient.setInsecure();
  #else
    espClient.setCACert(root_ca);      // enable this line and the the "certificate" code for secure connection
  #endif

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void display_heat_index(float HI){
  // Define the ranges of the heat index for the four classifications
    float low_range[2] = {60.0, 80.0};
    float moderate_range[2] = {80.0, 100.0};
    float high_range[2] = {100.0, 120.0};
    float very_high_range[2] = {120.0, 150.0};

    // Set up GPIO pins for servo motor control
    Servo myservo;
    myservo.attach(5);

    // Set the initial position of the servo motor
    myservo.write(0);

    // Determine which classification the heat index falls into and set the servo motor position
    float position;
    if (HI >= low_range[0] && HI < low_range[1]) {
        position = 0;  // Low position (midpoint of low_range)
    }
    else if (HI >= moderate_range[0] && HI < moderate_range[1]) {
        position = 45;  // Moderate position (midpoint of moderate_range)
    }
    else if (HI >= high_range[0] && HI < high_range[1]) {
        position = 90;  // High position (midpoint of high_range)
    }
    else if (HI >= very_high_range[0] && HI <= very_high_range[1]) {
        position = 135;  // Very high position (midpoint of very_high_range)
    }
    else {
        position = 180;  // Very low position (default if HI is outside all ranges)
    }

    // Set the position of the servo motor to the appropriate angle
    myservo.write(position);

    // Wait for a short time to allow the servo motor to move
    delay(100);

    // Stop the servo motor
    myservo.detach();
}

/*** Main Function ******/
void loop() {

  if (!client.connected()) reconnect(); // check if client is connected
  client.loop();

//read DHT11 temperature and humidity reading
  delay(dht.getMinimumSamplingPeriod());
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
    // Check if any reads failed
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

   // Calculate Heat Index using the formula you provided
  HI = (-42.379 + 2.04901523 * temperature + 10.14333127 * humidity - 0.22475541 * temperature * humidity
        - 0.00683783 * temperature * temperature - 0.05481717 * humidity * humidity + 0.00122874 * temperature * temperature * humidity
        + 0.00085282 * temperature * humidity * humidity - 0.00000199 * temperature * temperature * humidity * humidity);

  display_heat_index(HI);

  DynamicJsonDocument doc(1024);

  doc["deviceId"] = "NewID";
  doc["siteId"] = "IOT2023";
  doc["humidity"] = humidity;
  doc["temperature"] = temperature;
  doc["HeatIndex"] = HI;

  char mqtt_message[128];
  serializeJson(doc, mqtt_message);

  publishMessage("esp8266_data", mqtt_message, true);

  delay(5000);

}