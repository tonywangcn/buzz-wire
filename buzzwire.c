/***************************************************
 Clock v 8.0
 
 by Pavel Mihaylov
 Oct 2012
 ****************************************************/

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

Adafruit_7segment matrix = Adafruit_7segment();

//int pwr = A2;
//int grnd = A3;

//Buttons
int alarm_off_1 = 11;
int alarm_off_2 = 12;
int alarm_wire = 13;
int button_set_mins_in = 5;
int button_set_hrs_in = 8;
int button_settings_in = 10;
int beeper = 2;
int led_RGB_1 = 3;
int led_RGB_2 = 4;

//Constants
int brightness = 6; //Matrix initial brightness: 0(dimmed) - 15(bright)
int del = 160; //120 Delay betweeen screens when writing messages
int del2 = 320; //320 Delay betweeen "PAUSE" screens when writing messages
long int millis_left = 0;
int beep_delay = 7; //Length of a beep sound when a button is pressed
int beepOK = 0; //Variable checked whether it is an exact hour in order to produce two beeps: beepOK = 1 if minutes = 59, otherwise beepOK = 0;
int alarm_1_time = 2400; //Holds alarm time DO NOT CHANGE
int alarm = 1; //alarm status DO NOT CHANGE


long int offset_m = 0; //minutes setting offset

unsigned long time;
int h, m, s;
int h1, h2, m3, m4;
int a1h1, a1h2, a1m3, a1m4;

void setup() {
    pinMode(beeper, OUTPUT);
    pinMode(led_RGB_1, OUTPUT);
    pinMode(led_RGB_2, OUTPUT);
    pinMode(button_set_mins_in, INPUT);
    pinMode(button_set_hrs_in, INPUT);
    pinMode(button_settings_in, INPUT);
    pinMode(alarm_off_1, INPUT);
    pinMode(alarm_off_2, INPUT);
    pinMode(alarm_wire, INPUT);
    
    
    digitalWrite(beeper, LOW);
    digitalWrite(led_RGB_1, LOW);
    digitalWrite(led_RGB_2, LOW);
    
    
    
    Serial.begin(9600);
    Serial.println("7 Segment Backpack Test");
    
    matrix.begin(0x70);
    
    matrix.setBrightness(brightness);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
    
    
    //Write time
    Serial.print(" --- Time \n");
    writetime();
    delay(30);
    
    
    //Draw dots
    drawDots();
    delay(30);
    
    
    //Check whether to go into settings mode
    if(digitalRead(button_settings_in) == HIGH){
        beep();
        Serial.print("In settings \n");
        settings();
    }
    delay(30);
    
    
    //Now check if we have alarm acivated!
    gettime();
    //Serial.print("Before: Alarm = ");
    //Serial.print(alarm);
    //Serial.print("\n");
    //delay(30);
    if( time == alarm_1_time && alarm){
        //alarm = 0;
        alarm_beep();  //Play the beep
        Serial.print("NO MORE BEEP \n");
        delay(500);
    }
    else if(time != alarm_1_time) alarm = 1;
    //Serial.print("After: Alarm = ");
    //Serial.print(alarm);
    //Serial.print("\n");
    //Serial.print("NO ALARM \n");
    //delay(60);
    
    //Check brightness
    if (digitalRead(button_set_mins_in) == HIGH){ //check minutes button
        brightness++; //increase brightness
        beep();
        if( brightness > 15 ) { brightness = 15; delay(beep_delay); beep(); delay(beep_delay); beep(); }
        matrix.setBrightness(brightness);
        Serial.print("Brightness level changed:");
        Serial.print(brightness);
        Serial.print("\n");
        delay(150);
    }
    if (digitalRead(button_set_hrs_in) == HIGH){ //check hours button
        brightness--; //decrease brightness
        beep();
        if( brightness < 0 ) { brightness = 0; delay(beep_delay); beep(); delay(beep_delay); beep(); }
        matrix.setBrightness(brightness);
        Serial.print("Brightness level changed:");
        Serial.print(brightness);
        Serial.print("\n");
        delay(150);
    }
    
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Draw dots every second - On/Off
void drawDots(){
    gettime();
    
    if((s%2) == 1){
        matrix.drawColon(false);
        matrix.writeDisplay();
    }
    else{
        matrix.drawColon(true);
        matrix.writeDisplay();
    }
    
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int gettime(){
    
    time = millis();
    time += offset_m;
    
    h = int(time/3600000); //Get time in hours in variable h
    if(h > 23) do{ h -= 24; } while(h > 23); //If hours exceed 23, cunstract 24 until hours are 23 or less
    
    time = time%3600000;
    m = int(time/60000); //Get time in minutes in variable  m
    
    time = time%60000;
    s = int(time/1000); //Get time in seconds in variable  s
    
    millis_left = time;
    time = h*100 + m;
    
    //loop to produce two beeps at an exact hour i.e. XX:00
    if(m == 0 && beepOK){
        beep();
        delay(beep_delay*6);
        beep();
        beepOK = 0;
    }
    else if(m == 59) beepOK = 1;
    
    return time;
}


void writetime(){
    
    gettime();
    
    h1 = time/1000;
    time -= h1*1000;
    
    h2 = time/100;
    time -= h2*100;
    
    m3 = time/10;
    m4 = time%10;
    
    matrix.writeDigitNum(0, h1);
    matrix.writeDigitNum(1, h2);
    matrix.writeDigitNum(3, m3);
    matrix.writeDigitNum(4, m4);
    matrix.writeDisplay();
}




void settings(){
    delay(500);
    
    print_set(); //Print "Set" on the screen
    
    //Firstly set the time
    do{ set_hrs_mins(); }
    while (digitalRead(button_settings_in) == LOW);
    beep();
    
    
    print_set_alrarm_1(); //Print "Set" on the screen
    
    //Then set Alarm 1
    do{ set_alarm_1(); }
    while (digitalRead(button_settings_in) == LOW);
    beep();
    
    
    print_out();
    delay(500);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Function to set time offset in minutes and hours
void set_hrs_mins(){
    
    
    if (digitalRead(button_set_mins_in) == HIGH){ //check minutes button
        offset_m = offset_m + 60000; //add one minute to offset
        
        beep();
        Serial.print("Increment minutes");
        Serial.print(offset_m);
        Serial.print("\n");
        delay(200);
    }
    if (digitalRead(button_set_hrs_in) == HIGH){ //check hours button
        offset_m = offset_m + 60*60000; //add 60*1 minutes to offset the time
        
        beep();
        Serial.print("Increment hours");
        Serial.print(offset_m);
        Serial.print("\n");
        delay(200);
    }
    
    beepOK = 0;
    gettime();
    
    //Now make the display blink while setting the time.
    
    if((millis_left%1000) > 500){ //0.5 seconds lit up
        writetime();
        matrix.drawColon(true);
        matrix.writeDisplay();
    }
    else{
        matrix.drawColon(false); //0.5 seconds lit off
        matrix.writeDigitRaw(0,B00000000); //" "
        matrix.writeDigitRaw(1,B00000000); //" "
        matrix.writeDigitRaw(3,B00000000); //" "
        matrix.writeDigitRaw(4,B00000000); //" "
        matrix.writeDisplay();
    }
    
    
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Function to set Alarm time 1
void set_alarm_1(){
    
    
    
    if (digitalRead(button_set_mins_in) == HIGH){ //check minutes button
        if(alarm_1_time==2400){ alarm_1_time = 0; }
        alarm_1_time++; //add one minute to offset
        
        beep();
        Serial.print("Increment alarm minutes ");
        Serial.print(alarm_1_time);
        Serial.print("\n");
        delay(200);
    }
    
    if (digitalRead(button_set_hrs_in) == HIGH){ //check hours button
        if(alarm_1_time==2400){ alarm_1_time = 0; }
        alarm_1_time += 100; //add 60*1 minutes to offset the time
        
        beep();
        Serial.print("Increment alarm hours ");
        Serial.print(alarm_1_time);
        Serial.print("\n");
        delay(200);
    }
    
    if(alarm_1_time%100 == 60){ //alarm time is now maybe 1260, but it should not be!
        alarm_1_time += 40;       //alarm time goes to 1300
        Serial.print("Alarm 1 time corrected to +40\n");
    }
    if(alarm_1_time > 2359){ //alarm time is now 2401, but it should not be!
        alarm_1_time = 2400;       //alarm time goes to 0000
        Serial.print("Alarm 1 time corrected to 0000\n");
    }
    
    Serial.print("Alarm 1 time: ");
    Serial.print(alarm_1_time);
    Serial.print("\n");
    
    
    
    gettime();
    
    //Now make the display blink while setting the Alarm 1 time.
    
    if((millis_left%1000) > 500){ //0.5 seconds lit up
        if(alarm_1_time == 2400){ //If alarm is deactivated from user
            matrix.drawColon(false);
            matrix.writeDigitRaw(0,B01000000); //"-"
            matrix.writeDigitRaw(1,B01000000); //"-"
            matrix.writeDigitRaw(3,B01000000); //"-"
            matrix.writeDigitRaw(4,B01000000); //"-"
            matrix.writeDisplay();
        }
        else{
            a1h1 = alarm_1_time/1000;
            a1h2 = ( alarm_1_time - a1h1 * 1000 ) / 100;
            a1m3 = ( alarm_1_time - a1h1 * 1000 - a1h2 * 100 ) / 10;
            a1m4 = alarm_1_time % 10;
            
            matrix.drawColon(true);
            matrix.writeDigitNum(0, a1h1);
            matrix.writeDigitNum(1, a1h2);
            matrix.writeDigitNum(3, a1m3);
            matrix.writeDigitNum(4, a1m4);
            matrix.writeDisplay();
        }
    }
    else{
        matrix.drawColon(false); //0.5 seconds lit off
        matrix.writeDigitRaw(0,B00000000); //" "
        matrix.writeDigitRaw(1,B00000000); //" "
        matrix.writeDigitRaw(3,B00000000); //" "
        matrix.writeDigitRaw(4,B00000000); //" "
        matrix.writeDisplay();
    }
    
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Beep-Beep Alarm

void alarm_beep(){
    int count0 = 0;
    do{
        count0++;
        int count = 0; //cut the beep at some point
        writetime();
        drawDots();
        delay(20);
        digitalWrite(led_RGB_1, HIGH);
        digitalWrite(led_RGB_2, HIGH);
        
        if( count0%3 == 0 ){
            print_sad(); //Print a sad smile
            delay(300);
        }
        
        //Serial.print("In alarm - beep \n");
        beep();
        delay(100);
        beep();
        delay(100);
        beep();
        delay(150);
        
        
        
        if (digitalRead(alarm_off_1) == HIGH){
            //Serial.print("One 1 end touched :) \n");
            digitalWrite(led_RGB_1, LOW);
            print_smile();
            delay(500);
            do{
                
                //Now beep less
                if(count%5 == 0) {
                    beep();
                }
                count++;
                
                writetime();
                drawDots();
                
                if (digitalRead(alarm_off_2) == HIGH){
                    //Alarm off
                    //Serial.print("Other 2 end touched, alarm = 0 \n");
                    digitalWrite(led_RGB_2, LOW);
                    alarm = 0;
                    break;
                }
                //Serial.print("In while loop 1! \n");
                delay(100);
            }
            while(digitalRead(alarm_wire) == LOW || alarm == 0);
            delay(100);
            //Serial.print("While loop exited, now exit alarm \n");
            
        }
        else if (digitalRead(alarm_off_2) == HIGH){
            //Serial.print("One 2 end touched :) \n");
            digitalWrite(led_RGB_2, LOW);
            print_smile();
            delay(500);
            do{
                
                //Now beep less
                if(count%5 == 0) {
                    beep();
                }
                count++;
                
                writetime();
                drawDots();
                
                if (digitalRead(alarm_off_1) == HIGH){
                    //Alarm off
                    //Serial.print("Other 1 end touched, alarm = 0 \n");
                    digitalWrite(led_RGB_1, LOW);
                    alarm = 0;
                    break;
                }
                Serial.print("In while loop 2! \n");
                delay(100);
            }
            while(digitalRead(alarm_wire) == LOW || alarm == 0);
            delay(100);
            //Serial.print("While loop exited, now exit alarm \n");
            //exit;
        }
        
        
        
    }
    while( alarm != 0 );
    print_smile(); //Show a smile
    delay(700);
    print_alarm_off();
    
    //Serial.print("I said!!!!! Exit alarm loop: Alarm = ");
    //Serial.print(alarm);
    //Serial.print("\n");
}









// " " B00000000
//  S  B01101101
//  E  B01111001
//  t  B01111000
//  O  B00111111
//  u  B00011100
//  A  B01110111
//  l  B00111000 (letter)
//  r. B11010000
//  r  B01010000
//  1  B00000110 (digit)
//  H  B01110110
//  o  B01011100
//  ]  B00001111
//  [  B00111001
//  F  B01110001



//Print "Set Hour" on screen - Rolling
void print_set(){
    
    matrix.drawColon(false);
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDigitRaw(1,B00000000); //" "
    matrix.writeDigitRaw(3,B00000000); //" "
    matrix.writeDigitRaw(4,000000000); //" "
    matrix.writeDisplay();
    delay(del);
    
    //" "
    //" "
    //" "
    matrix.writeDigitRaw(4,B01101101); //"S"
    matrix.writeDisplay();
    delay(del);
    
    //" "
    //" "
    matrix.writeDigitRaw(3,B01101101); //"S"
    matrix.writeDigitRaw(4,B01111001); //"E"
    matrix.writeDisplay();
    delay(del);
    
    //" "
    matrix.writeDigitRaw(1,B01101101); //"S"
    matrix.writeDigitRaw(3,B01111001); //"E"
    matrix.writeDigitRaw(4,B01111000); //"t"
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B01101101); //"S"
    matrix.writeDigitRaw(1,B01111001); //"E"
    matrix.writeDigitRaw(3,B01111000); //"t"
    matrix.writeDigitRaw(4,B00000000); //" "
    matrix.writeDisplay();
    delay(del2);
    
    matrix.writeDigitRaw(0,B01111001); //"E"
    matrix.writeDigitRaw(1,B01111000); //"t"
    matrix.writeDigitRaw(3,B00000000); //" "
    matrix.writeDigitRaw(4,B01110110); //"H"
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B01111000); //"t"
    matrix.writeDigitRaw(1,B00000000); //" "
    matrix.writeDigitRaw(3,B01110110); //"H"
    matrix.writeDigitRaw(4,B01011100); //"o"
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDigitRaw(1,B01110110); //"H"
    matrix.writeDigitRaw(3,B01011100); //"o"
    matrix.writeDigitRaw(4,B00011100); //"u"
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B01110110); //"H"
    matrix.writeDigitRaw(1,B01011100); //"o"
    matrix.writeDigitRaw(3,B00011100); //"u"
    matrix.writeDigitRaw(4,B01010000); //"r"
    matrix.writeDisplay();
    delay(del2);
    
    matrix.writeDigitRaw(0,B01011100); //"o"
    matrix.writeDigitRaw(1,B00011100); //"u"
    matrix.writeDigitRaw(3,B01010000); //"r"
    matrix.writeDigitRaw(4,B00000000); //" "
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B00011100); //"u"
    matrix.writeDigitRaw(1,B01010000); //"r"
    matrix.writeDigitRaw(3,B00000000); //" "
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B01010000); //"r"
    matrix.writeDigitRaw(1,B00000000); //" "
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDisplay();
    delay(del*3);
    
    
}







void print_out(){ //Print "Out" on screen. Rolling.
    
    matrix.drawColon(false);
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDigitRaw(1,B00000000); //" "
    matrix.writeDigitRaw(3,B00000000); //" "
    matrix.writeDigitRaw(4,000000000); //" "
    matrix.writeDisplay();
    delay(del);
    
    //matrix.writeDigitRaw(0,B00000000); //" "
    //matrix.writeDigitRaw(1,B00000000); //" "
    //matrix.writeDigitRaw(3,B00000000); //" "
    matrix.writeDigitRaw(4,B00111111); //"O"
    matrix.writeDisplay();
    delay(del);
    
    //matrix.writeDigitRaw(0,B00000000); //" "
    //matrix.writeDigitRaw(1,B00000000); //" "
    matrix.writeDigitRaw(3,B00111111); //"O"
    matrix.writeDigitRaw(4,B00011100); //"u"
    matrix.writeDisplay();
    delay(del);
    
    //matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDigitRaw(1,B00111111); //"O"
    matrix.writeDigitRaw(3,B00011100); //"u"
    matrix.writeDigitRaw(4,B01111000); //"t"
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B00111111); //"O"
    matrix.writeDigitRaw(1,B00011100); //"u"
    matrix.writeDigitRaw(3,B01111000); //"t"
    matrix.writeDigitRaw(4,B00000000); //" "
    matrix.writeDisplay();
    delay(del2);
    
    matrix.writeDigitRaw(0,B00011100); //"E"
    matrix.writeDigitRaw(1,B01111000); //"t"
    matrix.writeDigitRaw(3,B00000000); //" "
    //matrix.writeDigitRaw(4,B00000000); //" "
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B01111000); //"t"
    matrix.writeDigitRaw(1,B00000000); //" "
    //matrix.writeDigitRaw(3,B00000000); //" "
    //matrix.writeDigitRaw(4,B00000000); //" "
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDisplay();
    delay(del*3);
}





//Print "Set Alr. 1" on screen - Rolling
void print_set_alrarm_1(){
    
    matrix.drawColon(false);
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDigitRaw(1,B00000000); //" "
    matrix.writeDigitRaw(3,B00000000); //" "
    matrix.writeDigitRaw(4,B00000000); //" "
    matrix.writeDisplay();
    delay(del);
    
    //" "
    //" "
    //" "
    matrix.writeDigitRaw(4,B01101101); //"S"
    matrix.writeDisplay();
    delay(del);
    
    //" "
    //" "
    matrix.writeDigitRaw(3,B01101101); //"S"
    matrix.writeDigitRaw(4,B01111001); //"E"
    matrix.writeDisplay();
    delay(del);
    
    //" "
    matrix.writeDigitRaw(1,B01101101); //"S"
    matrix.writeDigitRaw(3,B01111001); //"E"
    matrix.writeDigitRaw(4,B01111000); //"t"
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B01101101); //"S"
    matrix.writeDigitRaw(1,B01111001); //"E"
    matrix.writeDigitRaw(3,B01111000); //"t"
    matrix.writeDigitRaw(4,B00000000); //" "
    matrix.writeDisplay();
    delay(del2);
    
    matrix.writeDigitRaw(0,B01111001); //"E"
    matrix.writeDigitRaw(1,B01111000); //"t"
    matrix.writeDigitRaw(3,B00000000); //" "
    matrix.writeDigitRaw(4,B01110111); //"A"
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B01111000); //"t"
    matrix.writeDigitRaw(1,B00000000); //" "
    matrix.writeDigitRaw(3,B01110111); //"A"
    matrix.writeDigitRaw(4,B00111000); //"l"
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDigitRaw(1,B01110111); //"A"
    matrix.writeDigitRaw(3,B00111000); //"l"
    matrix.writeDigitRaw(4,B11010000); //"r."
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B01110111); //"A"
    matrix.writeDigitRaw(1,B00111000); //"l"
    matrix.writeDigitRaw(3,B11010000); //"r."
    matrix.writeDigitRaw(4,B00000000); //" "
    matrix.writeDisplay();
    delay(del2);
    
    matrix.writeDigitRaw(0,B00111000); //"l"
    matrix.writeDigitRaw(1,B11010000); //"r."
    matrix.writeDigitRaw(3,B00000000); //" "
    matrix.writeDigitRaw(4,B00000110); //"1"
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B11010000); //"r."
    matrix.writeDigitRaw(1,B00000000); //" "
    matrix.writeDigitRaw(3,B00000110); //"1"
    matrix.writeDigitRaw(4,B00000000); //" "
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDigitRaw(1,B00000110); //"1"
    matrix.writeDigitRaw(3,B00000000); //" "
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B00000110); //"1"
    matrix.writeDigitRaw(1,B00000000); //" "
    matrix.writeDisplay();
    delay(del2);
    
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDisplay();
    delay(del*3);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Print "Alarm Off"

void print_alarm_off(){
    matrix.drawColon(false);
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDigitRaw(1,B00000000); //" "
    matrix.writeDigitRaw(3,B00000000); //" "
    matrix.writeDigitRaw(4,B00000000); //" "
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDigitRaw(1,B00000000); //" "
    matrix.writeDigitRaw(3,B00000000); //" "
    matrix.writeDigitRaw(4,B01110111); //"A"
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDigitRaw(1,B00000000); //" "
    matrix.writeDigitRaw(3,B01110111); //"A"
    matrix.writeDigitRaw(4,B00111000); //"l"
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDigitRaw(1,B01110111); //"A"
    matrix.writeDigitRaw(3,B00111000); //"l"
    matrix.writeDigitRaw(4,B11010000); //"r."
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B01110111); //"A"
    matrix.writeDigitRaw(1,B00111000); //"l"
    matrix.writeDigitRaw(3,B11010000); //"r."
    matrix.writeDigitRaw(4,B00000000); //" "
    matrix.writeDisplay();
    delay(del2);
    
    matrix.writeDigitRaw(0,B00111000); //"l"
    matrix.writeDigitRaw(1,B11010000); //"r."
    matrix.writeDigitRaw(3,B00000000); //" "
    matrix.writeDigitRaw(4,B00111111); //"O"
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B11010000); //"r."
    matrix.writeDigitRaw(1,B00000000); //" "
    matrix.writeDigitRaw(3,B00111111); //"O"
    matrix.writeDigitRaw(4,B01110001); //"F"
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDigitRaw(1,B00111111); //"O"
    matrix.writeDigitRaw(3,B01110001); //"F"
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(0,B00111111); //"O"
    matrix.writeDigitRaw(1,B01110001); //"F"
    matrix.writeDigitRaw(4,B00000000); //" "
    matrix.writeDisplay();
    delay(del2);
    
    matrix.writeDigitRaw(0,B01110001); //"F"
    matrix.writeDigitRaw(3,B00000000); //" "
    matrix.writeDisplay();
    delay(del);
    
    matrix.writeDigitRaw(1,B00000000); //" "
    matrix.writeDisplay();
    delay(del);  
    
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDisplay();
    delay(del*3); 
    
}






/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Print :]
void print_smile(){
    
    matrix.drawColon(true);
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDigitRaw(1,B00000000); //" "
    matrix.writeDigitRaw(3,B00001111); //"]"
    matrix.writeDigitRaw(4,B00000000); //" "
    matrix.writeDisplay();
    delay(del*2);
    
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Print :[ rolling
void print_sad(){
    
    matrix.drawColon(true);
    matrix.writeDigitRaw(0,B00000000); //" "
    matrix.writeDigitRaw(1,B00000000); //" "
    matrix.writeDigitRaw(3,B00111001); //"["
    matrix.writeDigitRaw(4,B00000000); //" "
    matrix.writeDisplay();
    delay(del*2);
    
}





//Button press beep function
void beep(){
    digitalWrite(beeper, HIGH);
    delay(beep_delay);
    digitalWrite(beeper, LOW);
}
