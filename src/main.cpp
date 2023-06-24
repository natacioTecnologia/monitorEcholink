#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <avr/wdt.h>

#define sensorPin A2
#define dataTemp 2
#define fanOut (1<<PD3)
#define tx     (1<<PB5)

#define setBit(Y,bit_x)  Y = (1<<bit_x)
#define clearBit(Y,bit_x) Y &= ~(1<<bit_x)

LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display
OneWire oneWire(dataTemp);
DallasTemperature sensor (&oneWire);

int sensorValue = 0;
char txFlag = 'R' ;
char protect = 0;
float tempProt = 52.1;

void setDuty(int duty);
float getVoltage();
void cooler(float temp);
void event();
void status();

void setup()
{
  wdt_enable(WDTO_8S);

  DDRB |= tx;           //Pino 13 como saida 
  clearBit(PORTB,PB5);        //Pino 13 em 0

  DDRD |= (1<<PD3);      //Pino 3 como saida 
  PORTD &= ~(1<<PD3);    //Pino 3 em 0

  TCCR2A = 0xA3;        //PWM FAST
  TCCR2B = 0x02;        //SET Prescaller

  delay(100);
  lcd.init();          // initialize the lcd 
  lcd.backlight(); // Start up the library
  sensor.begin();
  Serial.begin(9600);

   // clear the screen
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("Echolink");
  lcd.setCursor(5,1);
  lcd.print("PU6NPC");
  delay(500);

  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Teste Coller");

 for(int x =0; x<= 100; x++){
    setDuty(x);
    lcd.setCursor(7,1);
    lcd.print(x);
    delay(10);
 }
  delay(100);
  setDuty(0);
}
 
void loop(){  
  wdt_reset();
  sensor.requestTemperatures();
  float tempC = sensor.getTempCByIndex(0);
  cooler(tempC);
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print(tempC);
  lcd.setCursor(10,0);
  lcd.print("C");
  status();
  if (protect == 0)
  {
    lcd.setCursor(4,1);
    lcd.print(getVoltage());
    lcd.setCursor(10,1);
    lcd.print("V");
  }else{
    lcd.setCursor(4,1);
    lcd.print("Protect...");
  }
}


void setDuty(int d){
  int static duty;
  duty = map(d, 0, 100, 0, 255 );
  OCR2B = duty;
}

float getVoltage(){
  float v =0.0 ;
  float media[6];

  sensorValue = analogRead(sensorPin);

  for (int i = 0; i <= 5; i++)
  {
    media[i] = (5.0 * sensorValue) / 162;
  }

  for (int i = 0; i <= 5; i++){
    v = v + media[i];
  }

 return v / (5+1);
}

void cooler(float temp){
  if(temp <= 28.5){
    setDuty(0);
  }else if(temp >= 33.4 && temp < 40.0){
     setDuty(70);
     if (protect == 1)
     {
      protect = 0;
     }    
  }else if(temp >= 40.0 && temp < 60.0){
     setDuty(100);
  }

  if(temp >= tempProt){
    protect = 1;
  }
}

void serialEvent(){
  while (Serial.available()) {
    char buffer = (char)Serial.read(); 
    if(buffer != 'C'){
      if(buffer != txFlag)
        txFlag = buffer;
    } 
  }
  txFlag == 'T' && protect != 1 ? setBit(PORTB,PB5) : clearBit(PORTB,PB5);
}

void status(){
 if(txFlag == 'T' && protect != 1){
    lcd.setCursor(14,0);
    lcd.print("TX");
    wdt_reset();
  }else if(txFlag == 'R'){
    lcd.setCursor(14,0);
    lcd.print("RX");
  }
}