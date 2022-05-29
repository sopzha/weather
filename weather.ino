#include <wifi.h> //Connect to WiFi Network
    #include<string.h> // Used for some string handling and processing
    #include <tft_espi.h> // Graphics and font library for ST7735 driver chip
    #include <spi.h> // Used in support of TFT Display
    #include <arduinojson.h>
    
    TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
    
    //Some constants and some resources:
    const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
    const int GETTING_PERIOD = 2000; //periodicity of getting a number fact
    const int BUTTON_TIMEOUT = 1000; //button timeout in milliseconds
    const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
    const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
    char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
    char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
    char request_buffer2[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
    char response_buffer2[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
    
    char network[] = "MIT";
    char password[] = "";
    
    uint8_t channel = 1; //network channel on 2.4 GHz
    byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; //6 byte MAC address of AP you're targeting.
    
    const int BUTTON = 5; //connect to pin 5
    const int BUTTON_TOGGLE = 19; //connect to pin 19
    unsigned long timer; // used for starting millis() readings
    uint8_t draw_state;  //used for remembering state
    uint8_t temp_draw_state;
    uint8_t previous_value_toggle;  //used for remembering previous button
    uint8_t previous_value_select;  //used for remembering previous button
    
    // Store date and timestamp information to be printed on screen
    char* hour_tok;
    char* minute_tok;
    char* date_tok;
    
    // Print statements on screen
    char print1 [30];
    char print2 [30];
    char print3 [30];
    char print4 [30];
    
    void setup() {
      tft.init(); //initialize the screen
      tft.setRotation(2); //set rotation for our layout
      tft.setTextSize(1); //default font size
      tft.fillScreen(TFT_BLACK); //fill background
      tft.setTextColor(TFT_GREEN, TFT_BLACK); //set color of font to green foreground, black background
      Serial.begin(115200); //begin serial comms
    
      timer = millis();
    
      int n = WiFi.scanNetworks();
      Serial.println("scan done");
      if (n == 0) {
        Serial.println("no networks found");
      } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
          Serial.printf("%d: %s, Ch:%d (%ddBm) %s ", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "");
          uint8_t* cc = WiFi.BSSID(i);
          for (int k = 0; k < 6; k++) {
            Serial.print(*cc, HEX);
            if (k != 5) Serial.print(":");
            cc++;
          }
          Serial.println("");
        }
      }
      delay(100); //wait a bit (100 ms)
    
      //if using regular connection use line below:
      WiFi.begin(network, password);
      //if using channel/mac specification for crowded bands use the following:
      //WiFi.begin(network, password, channel, bssid);
      uint8_t count = 0; //count used for Wifi check times
      Serial.print("Attempting to connect to ");
      Serial.println(network);
      while (WiFi.status() != WL_CONNECTED && count < 6) {
        delay(500);
        Serial.print(".");
        count++;
      }
      delay(2000);
      if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
        Serial.println("CONNECTED!");
        Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
                      WiFi.localIP()[1], WiFi.localIP()[0],
                      WiFi.macAddress().c_str() , WiFi.SSID().c_str());
        delay(500);
      } else { //if we failed to connect just Try again.
        Serial.println("Failed to Connect :/  Going to restart");
        Serial.println(WiFi.status());
        ESP.restart(); // restart the ESP (proper way)
      }
    
      //sets IO pin 5 and 19 as inputs which default to a 3.3V signal when unconnected and 0V when the switch is pushed
      pinMode(BUTTON, INPUT_PULLUP);
      pinMode(BUTTON_TOGGLE, INPUT_PULLUP);
      // Initialize display state to 0 (i.e. home screen)
      draw_state = 0;
      // Initialize toggle marker to first state position
      temp_draw_state = 1;
      // Initialize toggle and select button state to 1
      previous_value_toggle = 1;
      previous_value_select = 1;
    }
    
    // Format print statements on screen, limiting to 4 printouts at a time
    void print_out(char* new_print) {
      if (strlen(print4) == 0) {
        for (int i = 0; i < strlen(new_print); i++) {
          print4[i] = new_print[i];
        }
      }
      else if (strlen(print3) == 0) {
        for (int i = 0; i < strlen(new_print); i++) {
          print3[i] = new_print[i];
        }
      }
      else if (strlen(print2) == 0) {
        for (int i = 0; i < strlen(new_print); i++) {
          print2[i] = new_print[i];
        }
      }
      else if (strlen(print1) == 0) {
        for (int i = 0; i < strlen(new_print); i++) {
          print1[i] = new_print[i];
        }
      }
      else {
        for (int i = 0; i < sizeof(print4); i++) {
          print4[i] = '\0';
        }
        for (int i = 0; i < sizeof(print3); i++) {
          print4[i] = print3[i];
          print3[i] = '\0';
        }
        for (int i = 0; i < sizeof(print2); i++) {
          print3[i] = print2[i];
          print2[i] = '\0';
        }
        for (int i = 0; i < sizeof(print1); i++) {
          print2[i] = print1[i];
          print1[i] = '\0';
        }
        for (int i = 0; i < strlen(new_print); i++) {
          print1[i] = new_print[i];
        }
    
      }
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 10, 2);
      if (strlen(print1) > 0) {
        tft.println(print1);
      }
      if (strlen(print2) > 0) {
        tft.println(print2);
      }
      if (strlen(print3) > 0) {
        tft.println(print3);
      }
      if (strlen(print4) > 0) {
        tft.println(print4);
      }
    
    }
    
    void loop() {
    
      // Make GET request to http://iesc-s3.mit.edu/esp32test/currenttime
      sprintf(request_buffer, "GET http://iesc-s3.mit.edu/esp32test/currenttime\r\n");
      strcat(request_buffer, "Host: iesc-s3.mit.edu\r\n"); //add more to the end
      strcat(request_buffer, "\r\n"); //add blank line!
      //submit to function that performs GET.  It will return output using response_buffer char array
      do_http_GET("iesc-s3.mit.edu", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
    
      // Make GET request to http://api.openweathermap.org/data/2.5/weather
      sprintf(request_buffer2, "GET http://api.openweathermap.org/data/2.5/weather?lat=42.360092&lon=-71.094162&appid=853fbb1c547ea91cf5ff9c91ec5b5a43\r\n");
      strcat(request_buffer2, "Host: api.openweathermap.org\r\n"); //add more to the end
      strcat(request_buffer2, "\r\n"); //add blank line!
      //submit to function that performs GET.  It will return output using response_buffer char array
      do_http_GET("api.openweathermap.org", request_buffer2, response_buffer2, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
    
      // Parse response buffer using delimiters and store date, hours, minutes in variables
      char* token = strtok(response_buffer, " :");
      int count = 1;
      while (token != NULL) {
        count++;
        if (count == 2) {
          date_tok = token;
        }
        if (count == 3) {
          hour_tok = token;
        }
        else if (count == 4) {
          minute_tok = token;
        }
        token = strtok(NULL, " :");
      }
    
      // Parse response buffer as json object
      StaticJsonDocument<1000> doc;
      char json[1000];
      strncpy(json, response_buffer2, sizeof(response_buffer2));
      deserializeJson(doc, json);
      double temp = doc["main"]["temp"];
      temp = (temp - 273.15) * (9.0 / 5.0) + 32;
      int humidity = doc["main"]["humidity"];
      int pressure = doc["main"]["pressure"];
      int visibility = doc["visibility"];
    
      // Get readindgs for toggle and select buttons
      uint8_t toggle_state = digitalRead(BUTTON_TOGGLE);
      uint8_t select_state = digitalRead(BUTTON);
    
      if (draw_state == 0) {
        // Show home screen with menu
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 10, 2);
        tft.println("OPTION MENU:");
        tft.println("Temperature");
        tft.println("Time");
        tft.println("Date");
        tft.println("Visibility");
        tft.println("Humidity");
        tft.println("Pressure");
        // Update position of circle to indicate toggle state
        tft.fillCircle(115, 19 + temp_draw_state * 16, 5, TFT_GREEN);
    
        // Update toggle marker if toggle button is pressed
        if (toggle_state == 0 and previous_value_toggle == 1) {
          temp_draw_state ++;
          temp_draw_state = temp_draw_state % 7;
          if (temp_draw_state == 0) {
            temp_draw_state ++;
          }
        }
    
        // Update state if select button is pressed
        if (select_state == 0 and previous_value_select == 1) {
          draw_state = temp_draw_state;
        }
      }
    
      // Print out temperature
      if (draw_state == 1) {
        char new_print[30];
        sprintf(new_print, "The temp is %f deg F", temp);
        print_out(new_print);
        draw_state = 0;
      }
      // Print out time
      if (draw_state == 2) {
        char new_print [30];
        sprintf(new_print, "The time is ", 15);
        strncat(new_print, hour_tok, 5);
        strncat(new_print, ":", 2);
        strncat(new_print, minute_tok, 5);
        print_out(new_print);
        draw_state = 0;
      }
      // Print out date
      if (draw_state == 3) {
        char new_print [30];
        sprintf(new_print, "The date is ", 15);
        strncat(new_print, date_tok, 15);
        print_out(new_print);
        draw_state = 0;
      }
      // Print out visibility
      if (draw_state == 4) {
        char new_print [30];
        sprintf(new_print, "The visibility is %d m", visibility);
        print_out(new_print);
        draw_state = 0;
      }
      // Print out humidity
      if (draw_state == 5) {
        char new_print [30];
        sprintf(new_print, "The humidity is %d percent", humidity);
        print_out(new_print);
        draw_state = 0;
      }
      // Print out pressure
      if (draw_state == 6) {
        char new_print [30];
        sprintf(new_print, "The pressure is %d hPa", pressure);
        print_out(new_print);
        draw_state = 0;
      }
      // Set old button value
      previous_value_toggle = toggle_state;
      previous_value_select = select_state;
    }
    
    /*----------------------------------
       char_append Function:
       Arguments:
          char* buff: pointer to character array which we will append a
          char c:
          uint16_t buff_size: size of buffer buff
    
       Return value:
          boolean: True if character appended, False if not appended (indicating buffer full)
    */
    uint8_t char_append(char* buff, char c, uint16_t buff_size) {
      int len = strlen(buff);
      if (len > buff_size) return false;
      buff[len] = c;
      buff[len + 1] = '\0';
      return true;
    }
    
    /*----------------------------------
       do_http_GET Function:
       Arguments:
          char* host: null-terminated char-array containing host to connect to
          char* request: null-terminated char-arry containing properly formatted HTTP GET request
          char* response: char-array used as output for function to contain response
          uint16_t response_size: size of response buffer (in bytes)
          uint16_t response_timeout: duration we'll wait (in ms) for a response from server
          uint8_t serial: used for printing debug information to terminal (true prints, false doesn't)
       Return value:
          void (none)
    */
    void do_http_GET(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial) {
      WiFiClient client; //instantiate a client object
      if (client.connect(host, 80)) { //try to connect to host on port 80
        if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
        client.print(request);
        memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
        uint32_t count = millis();
        while (client.connected()) { //while we remain connected read out data coming back
          client.readBytesUntil('\n', response, response_size);
          if (serial) Serial.println(response);
          if (millis() - count > response_timeout) break;
        }
        count = millis();
        if (serial) Serial.println(response);
        client.stop();
        if (serial) Serial.println("-----------");
      } else {
        if (serial) Serial.println("connection failed :/");
        if (serial) Serial.println("wait 0.5 sec...");
        client.stop();
      }
    }
