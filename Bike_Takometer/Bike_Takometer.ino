#include <SimpleRotary.h>
#include <SPI.h>
#include <Wire.h>
#include <Preferences.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SW 12
#define CLK 14
#define DT 27

#define SCL 22
#define SDA 21

#define SENS 33

#define MAX_RADIOUS 50

SimpleRotary rotary(CLK, DT, SW);

Adafruit_SSD1306 display(128, 64);

Preferences pref;

byte rotation = 0;
byte button = 0;
int MAX_ROTATION = 0; 

//curr_menu indicate the current selected menu to display
int curr_menu = 0;
int prev_menu = 0;
//curr_menu_entry indicate the current selected menu entry
int curr_menu_entry = 0; 
//int prev_menu_entry = -1;

int num_menu_entrys[] = {3, 2};
String main_menu_entry[3] = {"START", "STATISTICS", "SETTINGS"};
int main_menu_pos[3] = {2, 1, 3};
String statistics_menu_entry[2] = {"TEST", "BACK"};
int statistics_menu_pos[2] = {3, 0};

double velocity = 0.0;
float byke_radious = 0.0;
float byke_r_m = 0.0;
float byke_c = 0.0;
float radious = 32.5;
int start_time = 0;
int end_time = 0;
float t = 0;
int reset_time = 0;

void IRAM_ATTR calcSpeed(){
  if((millis()-start_time)>100){
    end_time = millis()-start_time;
    start_time = millis();
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin(SDA, SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  pinMode(SENS, INPUT);
  pref.begin("data", false);
  byke_radious = pref.getFloat("byke_radious");
  byke_r_m = byke_radious/100;
  byke_c = byke_r_m*2*3.14;
  pref.end();
  display.display();
  delay(2000);
  display.clearDisplay();
  attachInterrupt(digitalPinToInterrupt(SENS), calcSpeed, RISING);
  start_time = millis();
  reset_time = millis();
}

void loop() {
  //sens_status = digitalRead(SENS);
  //if(sens_status == HIGH)Serial.println(sens_status);
  encoderStatus();// read the status of the encoder to navigate on menu
  menu();
  //delay(10);
}

void encoderStatus(){
  // 0 = not turning, 1 = CW, 2 = CCW
  rotation = rotary.rotate();
  button = rotary.push();
  switch(rotation){
    case 1:
      Serial.println("CW");
      if(curr_menu_entry < MAX_ROTATION-1)curr_menu_entry++;
      else curr_menu_entry = 0;
      Serial.println(curr_menu_entry);
      break;
    case 2:
      Serial.println("CCW");
      if(curr_menu_entry > 0)curr_menu_entry--;
      else curr_menu_entry = MAX_ROTATION - 1;
      Serial.println(curr_menu_entry);
      break;
  }
  if(button == 1){
    Serial.println("Button");
  }
}

void menu(){
  if(curr_menu != prev_menu)display.clearDisplay();
  switch(curr_menu){
    case 0://main menu
      prev_menu = curr_menu;
      MAX_ROTATION = num_menu_entrys[curr_menu];
      display.setTextSize(2);
      display.setCursor(0,0);
      writeEntrys(main_menu_entry, num_menu_entrys[curr_menu]);
      button_pressed(main_menu_pos);
      break;
    case 1://Statistics menu
      prev_menu = curr_menu;
      MAX_ROTATION = num_menu_entrys[curr_menu];
      display.setTextSize(2);
      display.setCursor(0,0);
      writeEntrys(statistics_menu_entry, num_menu_entrys[curr_menu]);
      button_pressed(statistics_menu_pos);
      break;
    case 2://speed display
      prev_menu = curr_menu;
      t = end_time/1000.0;
      if(end_time > 0){
        reset_time = millis();
        velocity = byke_c/t*3.6;
        end_time = 0;
      }
      //Serial.println(reset_time);
      if((millis()-reset_time)>1500){
        reset_time = millis();
        velocity = 0;
      }
      //Serial.println(t);
      display.setTextSize(4);
      display.setCursor(0,0);
      display.setTextColor(WHITE, BLACK);
      display.println(velocity);
      display.setTextSize(2);
      display.print("km/h");
      display.display();
      if(button == 1){
        curr_menu = 0;
      }
      break;
    case 3://settings menu
      prev_menu = curr_menu;
      setWheel();
      display.setTextSize(1);
      display.setCursor(0,0);
      display.setTextColor(WHITE, BLACK);
      display.println("CURRENT RADIOUS");
      display.print(byke_radious);
      display.println(" cm");
      display.println("SET RADIOUS");
      display.print(radious);
      display.println(" cm");
      display.print("\nPUSH BUTTON TO SAVE");
      display.display();
      if(button == 1)saveWheel();
      break;
    default:
      curr_menu = 0;
  }
}

void button_pressed(int *entrys){
  if(button == 1){
    curr_menu = entrys[curr_menu_entry];
    Serial.println(curr_menu);
    curr_menu_entry = 0;
  }
}
void setWheel(){
  switch(rotation){
    case 1:
      Serial.println("CW");
      if(radious <= MAX_RADIOUS)radious = radious + 0.1;
      break;
    case 2:
      Serial.println("CCW");
      if(radious > 0) radious = radious - 0.1;
      break;
  }
}

void saveWheel(){
  byke_radious = radious;
  byke_r_m = byke_radious / 100;
  byke_c = byke_r_m * 2 * 3.14;
  pref.begin("data", false);
  pref.putFloat("byke_radious", byke_radious);
  pref.end();
  curr_menu = 0;
}



void writeEntrys(String *entrys, int k){
  for(int i = 0; i < k; i++){
    if(i == curr_menu_entry){
      display.setTextColor(BLACK, WHITE);
    }else{
      display.setTextColor(WHITE, BLACK);
    }
    display.println(entrys[i]);
  }
  display.display();
}
