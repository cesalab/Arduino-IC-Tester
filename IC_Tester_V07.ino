
/*
/  Tester for CMOS and TTl IC.
/  IC will be tested according to a test file.
/  Syntax of File
/  

$40161
16 
0C10110G10XXXXXV
0C10110G11LLLLLV
1C10110G00HHLHLV
1000001G11HHLHLV
1C00001G11HHHLLV
1C00001G11HHHHHV
1C00001G11LLLLLV
1C00001G11LLLHLV
1100001G11LLHLLV
1000001G11LLHLLV
0000001G11LLLLLV
1C00001G11XXXXXV

/  $  Start character before the part name (max 16 charachters after $)
/  Amount of pins of the IC
/  V = supply voltage
/  G = GND (fix by hardware)
/  0/1 = logical 0 or 1 as input to the IC 
/  L/H = logical 0 or 1 as output from the IC
/  X = IC output not relevant (signal wil be ignored)
/  C = Clock -> Trigger
/
/  general test routine:
/    1. seach file until a $ is found. Store the adress (this adress is used later on to print the name of the part)
/    2. set the IC as beeing "OK"
/    3. if part is set OK "OK" 
/    4. set all input signals 
/    5. trigger clock, if required
/    6. read outputs and compare to expected signals -> if different: IC -> NOK
/    7. if IC still OK, increment "where to find the description-adress"
/    8. find next $
/    9. if & -> end
*/

/*
/   Resistors
/   works well with 680 Ohms -> keeps the load to the Arduino low BUT way out of specifications of CMOS and TTL (still works well)
*/


/* Version 5
/  cleanup and translate all comments to english
*/


/* PINS
IC   IC    Pin
14   16    Arduino

8    9  =  5
9    10 =  4
10   11 =  3
11   12 =  2
12   13 =  13
13   14 =  A0
14   15 =  A1
     16 =  A2
     1  =  6
1    2  =  7
2    3  =  8
3    4  =  9
4    5  =  10
5    6  =  11
6    7  =  12
7    8  =  GND
*/


#include <Wire.h> //I2C library
#include <LiquidCrystal_I2C.h>

int Pin14[14]={7,8,9,10,11,12,A7,5,4,3,2,13,A0,A1};             // contains the correct allocation for a 14 Pins IC
int Pin16[16]={6,7,8,9,10,11,12,A7,5,4,3,2,13,A0,A1,A2};        // contains the correct allocation for a 16 Pins IC
int Pin20[20];                                                  // contains the correct allocation for a 20 Pins IC       -> not implemented
int Pin24[24];                                                  // contains the correct allocation for a 24 Pins IC       -> not implemented
int PinOut[24];                                                 // will be used for the correct sequence of pins          -> max. 24 Pins

char Signal[24];

int Taste=A6;                                                   // Swich to start test 

int Pin_max;                                                    // Max of Pins of the IC to be tested

boolean Part=LOW;                                               // to indicate if all test have been performed without error.

int List[10];                                                   // List of possible matches -> indicates the address of the part in the test file
  
char PrintPart[17];                                             // for printing the list of matching parts
int startP=0;

char daten_Tastatur[17];                                        // Keybord input
int Index_Tastatur=0;
int Zeichen_Tastatur;

boolean v_mode=LOW;


// Test-Data -> can be used if EEPROM is not wanted or only few parts need to be tested
char Daten[]="0001\r\n$4013\r\n14 \r\nLHC100G001CHLV\r\nHLC001G100CLHV\r\nLHC000G000CHLV\r\nHLC010G010CLHV\r\n$4066\r\n14 \r\n0HH000G0HH000V\r\n1HH100G1HH100V\r\n0LL011G0LL011V\r\n1HH111G1HH111V\r\n$4075\r\n14 \r\n00000LG0LL000V\r\n00110HG1HH100V\r\n10010HG0HH010V\r\n10110HG1HH110V\r\n11001HG0HH001V\r\n01101HG1HH101V\r\n11011HG0HH011V\r\n11111HG1HH100V\r\n$4081\r\n14 \r\n00LH11G11HL00V\r\n10LL10G10LL10V\r\n01LL01G01LL01V\r\n11HL00G00LH11V\r\n$4093\r\n14 \r\n00HH00G00HH00V\r\n10HH10G10HH10V\r\n01HH01G01HH01V\r\n11LL11G11LL11V\r\n&";


/*-----( Declare Constants )-----*/
/*-----( Declare objects )-----*/
// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address




void setup(){
int b;
byte c;
  
  Wire.begin();                                                       // initialise the connection
  
  Serial.begin(1200);                                                 // IMPORTANT -> set to 1200 when uploading data to EEPROM via Serial, for testing other speeds can be used e.g.9600
    
  lcd.begin(16,2);                                                    // initialize the lcd for 16 chars 2 lines, turn on backlight
  
  b=sizeof(Daten);

  Serial.println("IC-Tester V0.7");
  lcd.setCursor(0,0); 
  lcd.print("IC-Tester V0.7");
  
  Serial.print("Database V");
  lcd.setCursor(0,1); 
  lcd.print("Database V");
  
  PrintPart[startP]=(char)(i2c_eeprom_read_byte(0x50, startP));
  while((PrintPart[startP]>33)&&(startP<5)){                                                         // read the first 5 digits or until a non-printable byte
    startP++;
    PrintPart[startP]=(char)(i2c_eeprom_read_byte(0x50, startP));
  }  
  PrintPart[startP]=0x00;                                               // to make it printable via println
  Serial.println(PrintPart);
  lcd.print(PrintPart);                                                 // prints the database version e.g. 0004
  Serial.println("press 'v' to turn verbose mode on/off");
  Serial.println("press Key to start or enter part number (whithout prefix) - press ENTER...");
  Serial.println("press 'd' to load new EEPROM-Data -> paste EEPROM Data into dialog and press ENTER");   
}


//-----------//
// Main loop //
//-----------//

void loop(){
int j, i, h=0;
boolean run;
boolean start=LOW;
boolean found=LOW;
char c;

  while(((analogRead(Taste))<128)&&(start==LOW)){                       // until start button has been pressed
    if(Serial.available()){                                             // Part number was entered by Keyboard
      getKey();
      if((daten_Tastatur[0]=='v')||(daten_Tastatur[0]=='V')){
        if(v_mode==LOW){
          v_mode=HIGH;
          Serial.println("verbose mode on");
          daten_Tastatur[0]=0x00;
        }
        else{
          v_mode=LOW;
          Serial.println("verbose mode off");
          daten_Tastatur[0]=0x00;
        }
      }
      else if((daten_Tastatur[0]=='d')||(daten_Tastatur[0]=='D')){
        Serial.println("EEPROM Programmer Mode");
        Serial.println("Please paste in all data and press ENTER");
        while(!Serial.available()); // wait for data
        lcd.setCursor(0,1);
        lcd.print("write Database..");
        TransferToEEPROM();
        lcd.setCursor(0,1);
        lcd.print("done!           ");
        Serial.println("EEPROM updated");
        daten_Tastatur[0]=0x00;
      }
      else
        start==HIGH;
    }
  }
  
  delay(10);
  
  while(((analogRead(Taste))>128)&&(start==LOW));                       // until start button has been pressed
  
  run=HIGH;
  
  Serial.println("Test starting...");
  
  lcd.clear();
  lcd.setCursor(0,0);                                                 
  lcd.print("testing...");

  Part=HIGH;                                                            // to start with
  j=0;
  i=0;

  while(run==HIGH){
    c=(char)(i2c_eeprom_read_byte(0x50, j));  
    
    while((c!='$')&&(c!='&')){                                          // forward until a part description occurs or End of File 
      j++;
      c=(char)(i2c_eeprom_read_byte(0x50, j));
    }
    
    j++;                                                                // to get behind the $ mark
    
    if(start==HIGH){                                                    // test if part number == entered number
      while(found==LOW){
        found=HIGH;                                                     // assuming the IC was found    
        for(i=0;i<sizeof(daten_Tastatur);i++){
          if(daten_Tastatur[i]!=(char)(i2c_eeprom_read_byte(0x50, j+i)))
            found=LOW;                                                 // ... but it was not
        }
        if(found==LOW){                                                // search for the next part ID  
          while((c!='$')&&(c!='&')){
            j++;
            c=(char)(i2c_eeprom_read_byte(0x50, j));
          }
          if(c=='&'){                                                  // End of file
            found=HIGH;                                                // exit the loop
            run=LOW;                                                   // prevent further test to be performed
          }
          else
            j++;                                                        // to be one position behind the $ mark 
        }
      }  
      start=LOW;                                                       // do not re-enter the loop 
    }                                                                  // here start=LOW, found=HIGH, run=HIGH/LOW
    
    if(run==HIGH){                                                     // if it is still high
                                                                       
      List[h]=j;                                                       // take over adress of the part description
      if(v_mode==HIGH)
        Serial.print("testing: ");
      
      lcd.setCursor(0,1); 
      
      c=(char)(i2c_eeprom_read_byte(0x50, j));  

      while(c>32){                                                      // as long as it is a character -> printing the name of the part
        j++;
        if(v_mode==HIGH)
          Serial.print(c);
        lcd.print(c);
        c=(char)(i2c_eeprom_read_byte(0x50, j));  
      }                                                                
      lcd.print(" ");
      if(v_mode==HIGH)
        Serial.println("");

      while(c<33){                                                      // search first digit of amount of pins
        c=(char)(i2c_eeprom_read_byte(0x50, j));                        
        j++;                                                            
      }                                                            

      Pin_max=(c-48)*10;                                                // char to int
 
      c=(char)(i2c_eeprom_read_byte(0x50, j));                          // second digit of amount of pins
      Pin_max=Pin_max+(c-48);
      
      j++;                                                              // to get over the last digit

      c=(char)(i2c_eeprom_read_byte(0x50, j));
      
      while(c<33){                                                    // search for the next line which should either be $ or &
        j++;
        c=(char)(i2c_eeprom_read_byte(0x50, j));                        
      }
      
      if(v_mode==HIGH){
        Serial.print("Pins: ");
        Serial.println(Pin_max);
      }
      if(Pin_max==14){
        for(i=0;i<14;i++)
          PinOut[i]=Pin14[i];
      }
      if(Pin_max==16){
        for(i=0;i<16;i++)
          PinOut[i]=Pin16[i];
      }
    
      while((c!='$') && (c!='&')){                                  // do until next Part_ID was found or End of File
        while((Part==HIGH) && (c!='$') && (c!='&')){  
          for(i=0; i<Pin_max; i++){                                 // Copy Dataset
            Signal[i]=(char)(i2c_eeprom_read_byte(0x50, j+i));    
          }
          Signal[i]=0x00;

          j=j+i;
          c=(char)(i2c_eeprom_read_byte(0x50, j)); 
          while(c<33){                                              // search for the next line which should either be $ or &
            j++;
            c=(char)(i2c_eeprom_read_byte(0x50, j));                        
          }

          Part=Part&&test();                                        // test 
        
        }
              
        if(Part==LOW){                                              // if an error occured count until next part or EOF 
          while((c!='$') && (c!='&')){
            j++;
            c=(char)(i2c_eeprom_read_byte(0x50, j));
          }  
        }
        c=(char)(i2c_eeprom_read_byte(0x50, j));
      }
      
      if(Part==HIGH){                                               // all test performed without error
        Serial.println("Gotcha!");
        h++;
        if(h>9)
          lcd.setCursor(14,1);
        else
          lcd.setCursor(15,1);
        lcd.print(h);
      }
      else{                                                         // at least one of the test had an error
        Part=HIGH;                                                  // set back and do not count h++ -> as long as h=0 there has been no part identified
      }
    }  
    
    if((c=='&')||(found==HIGH)){                                    // either end of file or just a single part should have been tested
      // Serial.println("end of testing");
      if(h>0){
        while(h>0){
          h--;
          for(i=0;i<16;i++){
            PrintPart[i]=(char)(i2c_eeprom_read_byte(0x50, List[h]+i));  
            if(PrintPart[i]<33){
              PrintPart[i]=0x00;                                    // set end of string for printf
              i=17;                                                 // escape
            }          
          }
          PrintPart[16]=0x00;                                       // make sure there is a 0x00 in all cases
          lcd.clear();
          lcd.setCursor(0,0);   
          lcd.print("possible match:");
          Serial.print("possible match: ");
          Serial.println(PrintPart);                                // print the part Name
          lcd.setCursor(0,1);                                       // Start at character 4 on line 0
          lcd.print(PrintPart);
          while((analogRead(Taste))<128);                           // wait until key is presses again.      
          delay(10);
          while((analogRead(Taste))>128);                           // wait until released  
        }
      }
      else{
        lcd.clear();
        lcd.setCursor(0,0); 
        lcd.print("no match found");
        Serial.println("no match found");
        while((analogRead(Taste))<128);                             // wait until key is presses again.      
        delay(10);
        while((analogRead(Taste))>128);                             // wait until released  

      }
      run=LOW;
      Serial.println("");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("press: start");
    
    }
  }
}



boolean test(void){  
boolean result=HIGH;
int i;  

  if(v_mode==HIGH){
    Serial.print("s: ");
    Serial.println(Signal);
  }
  for(i=0;i<Pin_max;i++){
    if(Signal[i]=='V'){
      digitalWrite(PinOut[i], HIGH);
    }
    else if(Signal[i]=='L'){
      digitalWrite(PinOut[i], LOW);
      pinMode(PinOut[i],INPUT_PULLUP);
    }
    else if(Signal[i]=='H'){
      digitalWrite(PinOut[i], LOW);
      pinMode(PinOut[i],INPUT_PULLUP);
    }
  }
  delay(2);
  
  // Set Signals
  
  for(i=0;i<Pin_max;i++){
    if(Signal[i]=='0'){
      pinMode(PinOut[i],OUTPUT);
      digitalWrite(PinOut[i], LOW);    
    }
    if(Signal[i]=='C'){
      pinMode(PinOut[i],OUTPUT);
      digitalWrite(PinOut[i], LOW);    
    }
    else if(Signal[i]=='1'){
      pinMode(PinOut[i],OUTPUT);
      digitalWrite(PinOut[i], HIGH);
    }
  }

// Trigger Clock
  for(i=0;i<Pin_max;i++){
    if(Signal[i]=='C'){
      pinMode(PinOut[i],INPUT_PULLUP);
    }
  }
  
  delay(10);                            // keep it HIGH for some time
  
  for(i=0;i<Pin_max;i++){               
    if(Signal[i]=='C'){
      pinMode(PinOut[i],OUTPUT);
      digitalWrite(PinOut[i], LOW);     // set it back LOW
    }
  }

// Read Outputs
  
  delay(2);
  if(v_mode==HIGH)
    Serial.print("r: ");
  for(i=0;i<Pin_max;i++){
    if((Signal[i]=='H')&& (digitalRead(PinOut[i])==LOW)){            // Should be HIGH but is LOW
      result=LOW;  
      if(v_mode==HIGH)
        Serial.print("L");
    }
    else if((Signal[i]=='L')&&(digitalRead(PinOut[i])==HIGH)){      // Should be LOW but is HIGH
      result=LOW;  
      if(v_mode==HIGH)
        Serial.print("H");
    }
    else{
      if(v_mode==HIGH)
        Serial.print(" ");
    }
  }
  if(v_mode==HIGH)
    Serial.println(" ");
  
  
  return result;                        // return it the test was OK

}


/*
boolean test_old(void){  
boolean result=HIGH;
int i;  
  // Set Vcc and GND
  Serial.print("set signals: ");
  Serial.println(Signal);
  for(i=0;i<Pin_max;i++){
    if(Signal[i]=='G'){
      pinMode(PinOut[i],OUTPUT);
      digitalWrite(PinOut[i], LOW);    
    }
    else if(Signal[i]=='V'){
      pinMode(PinOut[i],OUTPUT);
      digitalWrite(PinOut[i], HIGH);
    }
    else if(Signal[i]=='L'){
      digitalWrite(PinOut[i], LOW);
      pinMode(PinOut[i],INPUT);
      Serial.print("set ");
      Serial.print(i);
      Serial.println(" to INPUT");
    }
    else if(Signal[i]=='H'){
      digitalWrite(PinOut[i], LOW);
      pinMode(PinOut[i],INPUT);  
      Serial.print("set ");
      Serial.print(i);
      Serial.println(" to INPUT");
    }
  }
  delay(100);
// Set Signals
  for(i=0;i<Pin_max;i++){
    if(Signal[i]=='0'){
      pinMode(PinOut[i],OUTPUT);
      digitalWrite(PinOut[i], LOW);    
    }
    if(Signal[i]=='C'){
      pinMode(PinOut[i],OUTPUT);
      digitalWrite(PinOut[i], LOW);    
    }
    else if(Signal[i]=='1'){
      pinMode(PinOut[i],OUTPUT);
      digitalWrite(PinOut[i], HIGH);
    }
  }

  while(digitalRead(Taste)==LOW);         // until start button has been pressed
  delay(10);
  while(digitalRead(Taste)==HIGH);         // until start button has been pressed


// Trigger Clock
  for(i=0;i<Pin_max;i++){
    if(Signal[i]=='C'){
      digitalWrite(PinOut[i], HIGH);    // set it HIGH
    }
  }
  
  delay(10);                            // keep it HIGH for some time
  
  for(i=0;i<Pin_max;i++){               
    if(Signal[i]=='C'){
      digitalWrite(PinOut[i], LOW);     // set it back LOW
    }
  }
// Read Outputs
  
  
  delay(100);
  
  for(i=0;i<Pin_max;i++){
    if((Signal[i]=='H')&& (digitalRead(PinOut[i])==LOW)){            // Should be HIGH but is LOW
      result=LOW;  
      Serial.print(i);
      Serial.println(": LOW");
    }
    else if((Signal[i]=='L')&&(digitalRead(PinOut[i])==HIGH)){      // Should be LOW but is HIGH
      result=LOW;  
      Serial.print(i);
      Serial.println(": HIGH");

    }
  }
  return result;                        // return it the test was OK
}


*/

void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ) {
  int rdata = data;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.write(rdata);
  Wire.endTransmission();
}

// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length ) {
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddresspage >> 8)); // MSB
  Wire.write((int)(eeaddresspage & 0xFF)); // LSB
  byte c;
  for ( c = 0; c < length; c++)
    Wire.write(data[c]);
  Wire.endTransmission();
}


byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {
  byte rdata = 0xFF;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress,1);
  if (Wire.available()) rdata = Wire.read();
  return rdata;
}

// maybe let's not read more than 30 or 32 bytes at a time!
void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length ) {
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress,length);
  int c = 0;
  for ( c = 0; c < length; c++ )
    if (Wire.available()) buffer[c] = Wire.read();
}

void TransferToEEPROM(void){
byte c;
int i=0;
boolean fertig1=LOW;
  lcd.setCursor(0,1);
  lcd.print("                ");
  while(!fertig1){
    while(!Serial.available());
    c = Serial.read();
    if(c=='&'){                 // last character
      fertig1=HIGH;
    }
    i2c_eeprom_write_byte(0x50, i, c);
    delay(5);                   // wait for EEPROM to transfer Data
    i++;
    if(i%100==0){
      lcd.setCursor(0,1);
      lcd.print(i);
    }
  }

}


boolean getKey(void){
byte c;
boolean fertig1=LOW;
  while(!fertig1){
    while(!Serial.available());
    c = Serial.read();
    daten_Tastatur[Index_Tastatur]=c;
    if(c=='\n'){
      daten_Tastatur[Index_Tastatur]=0x00;
      Zeichen_Tastatur=Index_Tastatur-1;
      fertig1=HIGH;                                                 
    }
    Index_Tastatur++;
    if(Index_Tastatur>15){
      daten_Tastatur[16]=0x00;
      fertig1=HIGH;
      Serial.flush();
    }  
  }
  return fertig1;
}

