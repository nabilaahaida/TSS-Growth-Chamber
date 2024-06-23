#include <OneWire.h>
#include <DHT.h>

//#define DHT_Type DHT11
#define DHT_Type DHT22

#define Compressor 9
#define ColdWaterTemp 5
#define Chamber 4
#define PINLED 13
#define Radiator 3
#define Waterpump 8

String inString;
float waterTemp, chamberTemp, chamberHum, radTemp, water, RadTemp, tempdht, humdht;

OneWire waterTempHandler(ColdWaterTemp);
OneWire radTempHandler(Radiator);
DHT chamberDHT(Chamber, DHT_Type);

String endChar = String(char(0xff))+ String(char(0xff))+String(char(0xff)); //menyimpan karakter menggunakan kode ASCII
unsigned long previousMillis = 0;
const long measPeriod = 2000; 

void setup(void) {
  Serial.begin(9600); 
  Serial1.begin(9600); 
  Serial3.begin(115200);
  pinMode(Compressor, OUTPUT);
  digitalWrite(Radiator, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PINLED, OUTPUT);
  digitalWrite(Compressor, LOW); 
  pinMode(Waterpump, OUTPUT);
  digitalWrite(Waterpump, LOW);

  chamberDHT.begin();
  delay(100);
}

void loop(void) {
  waterTemp = getTemp(waterTempHandler);
    if (waterTemp > 0 && waterTemp < 80) {
      water = waterTemp;
    }
      
  radTemp = getTemp(radTempHandler);
    if (radTemp > 0 && radTemp < 80) {
      RadTemp = radTemp;
    }

  chamberHum = chamberDHT.readHumidity();
  chamberTemp = chamberDHT.readTemperature();
    if (chamberHum != 25.50 && chamberHum != NAN && chamberTemp != NAN && chamberTemp != 25.50) {
      tempdht = 0.9121*chamberTemp - 0.5532;
      humdht = chamberHum;
    }
    
  /* Communicating to LCD */
  if(Serial1.available()){
      String dataDisp="";
      delay(10);
      while(Serial1.available()){
        dataDisp += char(Serial1.read());
      }
      Serial.println(dataDisp);
      exeData(dataDisp);
  }

  unsigned long currentMillis = millis(); // store the current time
  if (currentMillis - previousMillis >= measPeriod) { // check if 2000ms passed
    previousMillis = currentMillis;   // save the last time you blinked the LED 
    Serial1.print("t0.txt=\"");
    Serial1.print(tempdht,2);
    Serial1.print("\"");
    Serial1.print(endChar);
    Serial1.print("t1.txt=\"");
    Serial1.print(humdht,2);
    Serial1.print("\"");
    Serial1.print(endChar);

    String data = String(water)+ "," + String(RadTemp) + "," + String(tempdht) + "," + String(humdht);
    Serial3.println(data);
    Serial.println(data);

    if(tempdht < 17){
    digitalWrite(Compressor, LOW);
    Serial.println ("Compressor OFF");
    } else if (tempdht > 18){
    digitalWrite(Compressor, HIGH);
    Serial.println ("Compressor ON");
    }

    if(water <= 5.5){
      digitalWrite(Waterpump, HIGH);
      Serial.println("Waterpump and Blower ON");
    }  
    else if (water > 14.0) {
      digitalWrite(Waterpump, LOW);
      Serial.println("Waterpump and Blower OFF"); 
    }
  }

  while(Serial3.available()) {
    char inChar = Serial3.read();
    inString += inChar;
    if (inChar == '\n') {
      Serial.println(inString);
      if (inString.indexOf("ON")>=0) {
        digitalWrite(PINLED, HIGH);
      }
      else if (inString.indexOf("OFF")>=0) {
        digitalWrite(PINLED, LOW);
      }
      inString = "";
    }
  }
  delay(100);
}

/*Function get temperature from sensor at compressor and heater*/
float getTemp(OneWire handler){
  byte data[12];
  byte addr[8];

  if ( !handler.search(addr)) {
      //no more sensors on chain, reset search
      handler.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  handler.reset();
  handler.select(addr);
  handler.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = handler.reset();
  handler.select(addr);
  handler.write(0xBE); // Read Scratchpad

  for (int i = 0; i < 9;  i++  ) { // we need 9 bytes
    data[i] = handler.read();
  }

  handler.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;

  return TemperatureSum;
}

/*Function to execute data from LCD*/
void exeData(String dataDisp){
  if(dataDisp =="LAMP-ON"){
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("LAMP ON");
  }
  if(dataDisp =="LAMP-OFF"){
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("LAMP OFF");
  }
}

  
    
