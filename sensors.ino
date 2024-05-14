void connectToWifi()
{
  IPAddress staticIP(192, 168, 1, 101); // ESP32 static IP
  IPAddress gateway(192, 168, 1, 1);    // IP Address of your network gateway (router)
  IPAddress subnet(255, 255, 255, 0);   // Subnet mask
  IPAddress primaryDNS(192, 168, 1, 1); // Primary DNS (optional)
  IPAddress secondaryDNS(0, 0, 0, 0);   // Secondary DNS (optional)

  WiFi.enableSTA(true);
  delay(500);

  WiFi.begin(wifi_network_ssid, wifi_network_password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
    }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    // Configuring static IP
    if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("Failed to configure Static IP");
    } else {
      Serial.println("Static IP configured!");
    }

    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

}

String readDSTemperatureC() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  if (tempC == -127.00) {
    Serial.println("Failed to read from DS18B20 sensor");
    return "--";
  } else {
    Serial.print("Temperature Celsius: ");
    Serial.println(tempC);
  }
  return String(tempC);
}

int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25;       // current temperature for compensation

// median filtering algorithm
int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0) {
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

void get_analogSampleTimepoint() {
  analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
  analogBufferIndex++;
  if (analogBufferIndex == SCOUNT) {
    analogBufferIndex = 0;
  }
}

String readTDS() {
  for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
    analogBufferTemp[copyIndex] = analogBuffer[copyIndex];

    // read the analog value more stable by the median filtering algorithm, and convert to voltage value
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 4096.0;

    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationCoefficient = 1.0 + 0.02 * (25 - 25.0);
    //temperature compensation
    float compensationVoltage = averageVoltage / compensationCoefficient;

    //convert voltage value to tds value
    tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;
  }
  tdsValue *= 2.5;
  Serial.print("Voltage: ");
  Serial.print(averageVoltage);
  Serial.print("V\t");
  Serial.print("TDS Value: ");
  Serial.print(tdsValue);
  Serial.println("ppm");
  return String(tdsValue);
}

//https://www.electroniclinic.com/esp32-ph-sensor-iot-ph-sensor-code-and-circuit-diagram/
String readPHLevel() {
  float calibration_value = 19.00f;  //20.24 - 0.7;
  int buffer_arr[10], temp = 0;
  unsigned long int avgval = 0;
  for (int i = 0; i < 10; i++)
  {
    buffer_arr[i] = analogRead(phPIN);
    delay(30);
  }
  for (int i = 0; i < 9; i++)
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buffer_arr[i] > buffer_arr[j])
      {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }
  for (int i = 2; i < 8; i++)
    avgval += buffer_arr[i];
  float volt = ((float)avgval / 6) * 3.3 / 4096.0;
  ph_act = -7.05 * volt + calibration_value;

  Serial.print("pH Val: ");
  Serial.print(ph_act);
  Serial.print("\tVolt: ");
  Serial.println(volt);
  return String(ph_act);
}

/* FILE FUNCTIONS */
void SD_Card_setup(String file_path) {
  SPI.begin(SCK, MISO, MOSI, SDPin);
  // see if the card is present and can be initialized:
  if (!SD.begin(SDPin)) {
    Serial.println("Card failed not present");
    watch = rtc.getTime("%d%B%Y %H:%M:%S");
    message = watch + " Card failed to initiate...";
  } else {
    watch = rtc.getTime("%d%B%Y %H:%M:%S");
    message = watch + " Logging initiated...";
    if (file_available(SD, file_path)) {
      appendFile(SD, filePath, message);
      Serial.println(filePath + " exists");
      //delay(1000);
    } else {
      writeFile(SD, filePath, message);
    }
  }
}

bool file_available(fs::FS &fs, String filePath) {
  File root = fs.open(filePath);
  if (!root) {
    Serial.printf("Failed to open %s\n", filePath);
    return false;
  }
  if (root.isDirectory()) {
    Serial.printf("Failed to open %s... Path given was a directory\n", filePath);
    return false;
  }
  watch = rtc.getTime("%d%B%Y %H:%M:%S");
  message = watch + " Logging initiated...\ntimeStamp,tempC,EC,pH";
  //  message = "File Found";
  Serial.println(message);
  return true;
}

void createDir(fs::FS &fs, String path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void writeFile(fs::FS &fs, String path, String message) {
  Serial.printf("Writing file: %s\n", path);
  File file = fs.open(path, FILE_WRITE);
  message = message + "\r\n";
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

String appendFile(fs::FS &fs, String path, String message) {
  File dataFile = SD.open(path, FILE_APPEND);
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(message);
    dataFile.close();
    // print to the serial port too:
    //Serial.println(message);
    return message;
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening " + path);
    return rtc.getTime("%d%B%Y %H:%M:%S") + "error opening " + path;
  }


}
