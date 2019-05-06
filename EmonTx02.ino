/*********************************************************************************************************************************************************************************************************************/
/* Sketch para el Emontx02                                                                                                                                                                                           */
/* Version:  5.1                                                                                                                                                                                                     */
/* Created:  23/02/2017                                                                                                                                                                                              */
/* Author:   Juan Taba Gasha                                                                                                                                                                                         */
/* Last Rev: 01/06/2018                                                                                                                                                                                              */
/*********************************************************************************************************************************************************************************************************************/
// Debido el tamaño del programa es necesario cambiar el bootloader del arduino nano a Optiboot (para liberar 1.5 KB de la flash), para descargar el programa seleccionar placa: Optiboot on 32-pin cpus CPU: ATMega328
// Función printf fue adicionada a la clase base Print del archivo de cabecera Print.h, esta función permite imprimir directamente sobre el display especificando parámetros de formato de salida del texto
// (Reemplazar el printf.h ubicado en C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino por la version modificada ubicada en la carpeta de librerias)
// La libreria RF69Mod.zip es una version reducida y mejorada de la jeelabs para disminuir el uso de memoria de programa y SRAM, ademas se adiciona la función de recepción de datos rf69_receiveStart(). 
//..................................................................................................................................................
//.  El Sketch usa 31902 bytes (98%) del espacio de almacenamiento de programa. El máximo es 32256 bytes.                                          .
//.  Las variables Globales usan 1154 bytes (56%) de la memoria dinámica, dejando 894 bytes para las variables locales. El máximo es 2048 bytes.   .
//..................................................................................................................................................

#define NO_PIN_STATE                              //Para no utilizar estado de variables de la libreria PinChangeInt y disminuir el uso de memoria de programa (30 Bytes)
#define NO_PIN_NUMBER  
#include <avr/wdt.h>                              
#include <PinChangeInt.h>
#include <U8g2lib.h>
#include <DHT.h>                 
#include <DallasTemperature.h>  
#include <OneWire.h>
#include <RTClib.h>
#include <Adafruit_NeoPixel.h>    
#include <RF69Mod.h>
#include <FiltersMod.h>

//pin 2,10,11,12 y 13 utilizados por RFM69
#define buzPin          0
#define ledTxPin        1
#define flowPin         3 
#define tempPin         4
#define termaResPin     5
#define termaOnPin      6
#define DHTPin          7
#define termaButtonPin  8 
#define selButtonPin    9
#define presPin        14
#define ledPin         15
#define neopixelPin    16 
#define pirPin         17
       
#define screenSaverTime   15                                                                        //Tiempo de espera en segundos para apagar la pantalla
#define font1             u8g2_font_fub14_tr                                                        //WH 21x16 1891 bytes
#define font2             u8g2_font_9x15_tr                                                         //WH 9x15  1246 bytes
#define font3             u8g2_font_7x13_tr                                                         //WH 6x13  1107 bytes
#define RF_freq           RF69_433MHZ                                                               //Frecuencia de transmision 433Mhz
#define FILTERSETTLETIME  5000                                                                      //Tiempo (ms) para estabilizar los filtros y empezar a enviar datos
#define filterFrequency   0.05                                                                      //Frecuencia de corte del filtro pasabajos en Hz
#define presInMax         7.18                                                                      //Presión de agua maxima de ingreso

U8G2_SSD1306_128X64_NONAME_2_HW_I2C display (U8G2_R0,U8X8_PIN_NONE);
Adafruit_NeoPixel neopixel = Adafruit_NeoPixel(1, neopixelPin, NEO_GRB + NEO_KHZ800);
DHT dht;
OneWire ourWire(tempPin);
DallasTemperature DS18b20(&ourWire);
RTC_DS1307 rtc;
DateTime now;
FilterOnePole lowpassFilter( LOWPASS, filterFrequency );

static const unsigned char hum_bmp[] /*size_16x17*/ U8X8_PROGMEM ={0x00, 0x00, 0x00, 0x10, 0x20, 0x10, 0x20, 0x28, 0x50, 0x28, 0x50, 0x44,0x88, 0x44, 0x88, 0x82, 0x04, 0x83, 0x04, 0x45, 0x02, 0x3A, 0x02, 0x02,0x02, 0x02, 0x02, 0x02, 0x04, 0x01, 0x88, 0x00, 0x70, 0x00 };
static const unsigned char temp_bmp[] /*size_16x16*/ U8X8_PROGMEM ={0x00, 0x60, 0x00, 0x90, 0x00, 0x88, 0x00, 0x44, 0x00, 0x22, 0x00, 0x15, 0x80, 0x0A, 0x40, 0x05, 0xA0, 0x02, 0x5C, 0x01, 0xA2, 0x00, 0x5D, 0x00, 0x5D, 0x00, 0x5D, 0x00, 0x22, 0x00, 0x1C, 0x00};
static const unsigned char flujo_bmp[] /*size_20x20*/ U8X8_PROGMEM ={0x00, 0xFE, 0x01, 0x00, 0xFE, 0x01, 0x00, 0x30, 0x00, 0x00, 0x30, 0x0C, 0x00, 0x30, 0x0C, 0x00, 0x78, 0x0C, 0xE0, 0xFF, 0x0F, 0xF0, 0xFF, 0x0F, 0x70, 0x78, 0x0C, 0x70, 0x00, 0x0C, 0x70, 0x00, 0x0C, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA8, 0x00, 0x00, 0xA8, 0x00, 0x00, 0x24, 0x01, 0x00, 0x24, 0x01, 0x00, 0x22, 0x02, 0x00, 0x22, 0x02, 0x00, 0x21, 0x04, 0x00};
static const unsigned char reloj_bmp[] /*size_16x16*/ U8X8_PROGMEM ={0xE0, 0x07, 0xF8, 0x1F, 0x3C, 0x3C, 0x0E, 0x70, 0x86, 0x60, 0x87, 0xE0, 0x83, 0xC0, 0x83, 0xC0, 0x83, 0xC7, 0x03, 0xC0, 0x07, 0xE0, 0x06, 0x60, 0x0E, 0x70, 0x3C, 0x3C, 0xF8, 0x1F, 0xE0, 0x07};
static const unsigned char flujo_test_bmp[] /*size_25x31*/ U8X8_PROGMEM ={0x3F, 0xCF, 0xF9, 0x01, 0xBF, 0xEF, 0xFB, 0x01, 0x8C, 0x61, 0x60, 0x00, 0x8C, 0xC7, 0x61, 0x00, 0x8C, 0x01, 0x63, 0x00, 0x8C, 0xEF, 0x63, 0x00, 0x0C, 0xCF, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x03, 0x00, 0x00, 0xFC, 0x03, 0x00, 0x00, 0x60, 0x30, 0x00, 0x00, 0x60, 0x30, 0x00, 0x00, 0x60, 0x30, 0x00, 0x00, 0xF0, 0x30, 0x00, 0xC0, 0xFF, 0x3F, 0x00, 0xE0, 0xFF, 0x3F, 0x00, 0xE0, 0xF0, 0x30, 0x00, 0xE0, 0x00, 0x30, 0x00, 0xE0, 0x00, 0x30, 0x00, 0xE0, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x01, 0x00, 0x00, 0x50, 0x01, 0x00, 0x00, 0x58, 0x02, 0x00, 0x00, 0x48, 0x02, 0x00, 0x00, 0x4C, 0x06, 0x00, 0x00, 0x46, 0x04, 0x00, 0x00, 0x42, 0x08, 0x00, 0x00};
static const unsigned char pres_bmp[] /*size_17x22*/ U8X8_PROGMEM ={0xC0, 0x07, 0x00, 0x30, 0x18, 0x00, 0x08, 0x20, 0x00, 0x04, 0x40, 0x00, 0x02, 0x80, 0x00, 0x12, 0x80, 0x00, 0x61, 0x00, 0x01, 0xE1, 0x01, 0x01, 0xC1, 0x00, 0x01, 0x41, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x04, 0x01, 0x02, 0x88, 0x00, 0x02, 0x80, 0x00, 0x04, 0x40, 0x00, 0x08, 0x20, 0x00, 0x30, 0x18, 0x00, 0xC0, 0x07, 0x00, 0x40, 0x04, 0x00, 0x40, 0x04, 0x00, 0x40, 0x04, 0x00, 0x40, 0x04, 0x00};

long long ackRev_buf;
unsigned long currentTime,cloopTime1,cloopTime2,cloopTime3,cloopTime4,volumenAgua,totalPulsosCal,totalPulsosAguaDia,totalPulsosAguaMes,lastChangeTime[] = {0,0,0},buzTime; 
volatile unsigned long totalPulsosAgua,termaTimeOn;
float humAmb,tempAmb,tempTerma,flujoAgua,presAgua;
volatile float flujoPulsos;
const int nodeIdEmonPi=5;                                                                            // Id del EmonPi
const int nodeId = 12;                                                                               // emonTx RFM12B nodo ID
const int netGroup = 210;                                                                            // emonTx RFM12B  grupo ID
unsigned int pirTimeOn,startcounter;
byte numPag = 0,timeInvDisplay=0,porcPresAgua,fechaMes,fechaDia,timeCountReset,txErr,buzBip,setTempMax;
char porcTempTerma;
bool displayInv=false,pirOn,ackRecv,StResTerma;
volatile bool termaModo,lastStTerma,termaBotonOn,resetTot,settled = false;                                                                                                                        
typedef struct { int dat0,dat1,dat2,dat3,dat4,dat5,dat6,dat7;byte dat8,dat9,dat10,dat11;unsigned long dat12;} PayloadTX;       
PayloadTX emontx; 
typedef struct {byte dataPos,setTempMax;unsigned long setVolumenAgua,setUnixTime;} PayloadRX;       
PayloadRX emonrx; 

/*Función para incrementar contadores de pulsos del sensor de flujo Modelo: YF-S201*/
void flow ()                                                                                                                          
{
     flujoPulsos++;                                                             //Contador para calculo de flujo
     totalPulsosAgua++;                                                         //Contador para calculo de volumen
}

/*Función para enviar datos a traves del modulo RF RFM69CW*/
void sendData ()
{
   emontx.dat0=startcounter;
   emontx.dat1=flujoAgua*100;
   emontx.dat2=tempTerma*10;
   emontx.dat3=tempAmb*10;
   emontx.dat4=humAmb*10;
   emontx.dat5=presAgua*100;
   emontx.dat6=porcTempTerma;
   emontx.dat7=porcPresAgua;
   emontx.dat8=pirOn;
   emontx.dat9=lastStTerma?(termaModo? 1:2):0;                                //Estado terma = 0:apagada, 1:encencida localmente, 2:encendida remotamente
   emontx.dat10=StResTerma;                                                    //Estado resistencia terma
   emontx.dat11=txErr;
   emontx.dat12=volumenAgua;
   
   if(!ackRecv){                                                              //Incrementa el contador de errores de Tx cuando no se recibe ACK
    ackRev_buf|=1;
    txErr++; 
   }
   if ((ackRev_buf&1LL<<59)) txErr--;                                        
   ackRev_buf=ackRev_buf<<1;
   ackRecv=0;
   rf69_sendNow (RF69_HDR_ACK|nodeId, &emontx, sizeof emontx);                //Envio de datos con petición de confirmación de recepción ACK y solo un intento de envio           
   digitalWrite (ledTxPin,!digitalRead(ledTxPin));                            //Led de la tarjeta arduino parpadea cuando se esta enviando datos
  
}

/*Función para cambiar de página al presionar el pulsador select*/
void changepage (){
 if (millis() - lastChangeTime[0] > 250){                                     //tiempo de espera 250ms para el evitar rebote del pulsador
   numPag++;
   pirTimeOn=0;                                                               //resetea temporizador de apagado de display
   if(numPag > 5) numPag=0;
   if(numPag==3) totalPulsosCal=totalPulsosAgua;                              //resetea el contador de volumen de agua para calibración  
   lastChangeTime[0]=millis(); 
 }
 resetTot=true;
}

/*Función para detectar que el boton local de encendido de terma fue pulsado*/
void localmode (){
  if (millis() - lastChangeTime[1] > 300){
    termaBotonOn=true;
    lastChangeTime[1]=millis(); 
  }
}

/*Función para detectar que la terma se ha encendido y ademas determina si fue encendido en forma local o remota */
void stTerma (){
  if (millis() - lastChangeTime[2] > 300){
    if(!lastStTerma){
      termaModo=(termaBotonOn ?1 :0);                        //termaModo => 0(Remoto)  1(Local)
      buzBip=(termaModo ?2 :3);                              //Emite 2 bip cuando enciende en forma local y 3 en remoto
      lastStTerma=1;
    }
    else{
      buzBip=1;
      lastStTerma=0; 
    }
    lastChangeTime[2]=millis();
  }
  termaTimeOn=now.unixtime();
  termaBotonOn=false;
  if(numPag==6)numPag=0;
  pirTimeOn=0;
}
 
/*Función para seleccionar el color del neopixel segun la temperatura del agua en la terma*/
 void setcolortemp (float t){
  if      (t>=0 && t<=(0.45*setTempMax))               neopixel.setPixelColor(0,neopixel.Color(0,0,255));     //Azul 
  else if (t>(0.45*setTempMax)&& t<=(0.6*setTempMax))  neopixel.setPixelColor(0, neopixel.Color(0,51,102));   //Celeste 
  else if (t>(0.6*setTempMax) && t<=(0.7*setTempMax))  neopixel.setPixelColor(0, neopixel.Color(0,255,0));    //Verde
  else if (t>(0.7*setTempMax) && t<=(0.8*setTempMax))  neopixel.setPixelColor(0, neopixel.Color(102,51,0));   //Naranja
  else if (t>(0.8*setTempMax) && t<=(0.9*setTempMax))  neopixel.setPixelColor(0, neopixel.Color(255,0,0));    //Rojo
  else if (t>(0.9*setTempMax))                         neopixel.setPixelColor(0, neopixel.Color(255,51,51));  //Fucsia
}

/*Función para encender o mantener encendido el display al detectar movimiento a traves del sensor PIR  */
void pir (){
  pirOn= digitalRead(pirPin);
  if(digitalRead(pirPin)) pirTimeOn=0;
  if(numPag==6)numPag=0;
}

/*Función para activar el piezo buzzer */
void buzzerbip (){
  if(buzBip){
    if((millis()-buzTime)>50 && !digitalRead(buzPin)){
      buzTime=millis(); 
      digitalWrite(buzPin,HIGH);
    }
    if((millis()-buzTime)>100 && digitalRead(buzPin)){
      digitalWrite(buzPin,LOW);
      buzTime=millis();
      buzBip--;
    }
  }
}

/*Función para dibujar barra de desplazamiento que indica la página actual*/
void drawbar(uint8_t x,uint8_t y,uint8_t p) {
  for (uint8_t i=0; i < 6; i++) {
  display.drawCircle((x+i*9),y,2,U8G2_DRAW_ALL);
  }    
  display.drawDisc((x+p*9),y,3,U8G2_DRAW_ALL);
}

/*Función para dibujar barras de señal de Tx*/
void drawTxBar (uint8_t x,uint8_t y,uint8_t Err){
  display.drawFrame(x,y+6,3,3);
  display.drawFrame(x+4,y+4,3,5);
  display.drawFrame(x+8,y+2,3,7);
  display.drawFrame(x+12,y,3,9);
  if(Err<59)display.drawVLine(x+1,y+7,1);
  if(Err<30)display.drawVLine(x+5,y+5,3);
  if(Err<11)display.drawVLine(x+9,y+3,5);
  if(Err<3)display.drawVLine(x+13,y+1,7);
}

/*Función para cargar imagen al display segun el número de página seleccionada*/
void loadpage(uint8_t numPag) {
  if (!displayInv&&!digitalRead(termaOnPin)) {                                    //Habilita el blinkeo del display y el neopixel cuando la terma esta encendida, 
    if (timeInvDisplay>(!digitalRead(termaResPin)?2:4)) {                         //Reduce la frecuencia de blinkeo cuando la resistencia de la terma se apaga
      displayInv=true;
      setcolortemp(tempTerma);
      timeInvDisplay=0;
    }
    else timeInvDisplay++;
  }
  else {
   displayInv=false; 
   neopixel.setPixelColor(0,0);
  }
  if(digitalRead(termaOnPin)&&(numPag==0||numPag==6)) setcolortemp(tempTerma);   //Enciende el neopixel cuando la terma esta apagada
  
  display.firstPage();                                                           //loop necesario para dibujar la pantalla por partes para reducir tamaño del buffer RAM  
  do{
  display.setDrawColor(1);
  switch (numPag) {
    case 0:                                                                      //Página principal muestra estado de la terma de agua
      display.setDrawColor(displayInv);
      display.drawBox(0, 0, 128, 64);
      display.setDrawColor(!displayInv);
      display.setFont(font2);
      display.setCursor(4,12);
      display.print(F("Terma"));
      display.setFont(font1);
      display.setCursor(75,20);
      if(tempTerma!=-127) display.printf(F("%2dC"),int(tempTerma));
      else display.print(F("--C"));
      display.setCursor(64,42);
      if(porcTempTerma!=-127) display.printf(F("%3d%%"),porcTempTerma);
      else{
        display.setCursor(77,42);
        display.print(F("-- %")); 
      }
      display.drawRFrame(4,16,47,24,7);
      
      if (!digitalRead(termaOnPin)){
        display.setCursor(12,35);
        display.print(F("ON"));
        display.setFont(font2);
        if (now.unixtime()-termaTimeOn<3600){
          display.setCursor(18,53);
          display.print((now.unixtime()-termaTimeOn)/60);
          display.print(F("m"));
        }
        else{
          display.setCursor(8,53);
          display.print((now.unixtime()-termaTimeOn)/3600);
          display.print(F("h"));
          display.print(((now.unixtime()-termaTimeOn)%3600)/60);
          display.print(F("m"));
        }
        display.setFont(font3);
        display.drawRFrame(94,49,12,15,3);
        display.setCursor(97,61);
        if (termaModo) display.print(F("L"));
        else display.print(F("R"));   
      }
      else {
        display.setCursor(8,35);
        display.print(F("OFF"));
        }
       drawTxBar(110,54,txErr);
       drawbar(41,60,numPag);
    break;
    case 1:                                                               //Página Hoy muestra temperatura y humedad ambiente
      display.setDrawColor(1);
      neopixel.setPixelColor(0,0);
      display.setFont(font2);
      display.setCursor(4,19);
      display.print(F("Hoy"));
      display.setFont(font1);
      display.setCursor(75,20);
      if(tempAmb!=-127) display.printf(F("%2dC"),int(tempAmb));
      else display.print(F("-- C"));
      display.setCursor(75,46);
      if (humAmb!=-127) display.printf(F("%2d%%"),int(humAmb));
      else display.print(F("-- %"));
      display.drawXBMP(50, 5,16,16,temp_bmp); 
      display.drawXBMP(50, 31,16,17,hum_bmp);  
      drawbar(41,60,numPag);
     break;
     case 2:                                                                //Página muestra flujo y totalizadores de agua
      display.setDrawColor(1);
      neopixel.setPixelColor(0,0);
      display.drawXBMP(8, 30,20,20,flujo_bmp); ;
      display.setFont(font1);
      display.setCursor(23,15);
      display.print(flujoAgua,1);
      display.print(F(" LPM"));
      display.setCursor(35,28);
      display.setFont(font3);
      display.printf(F("Dia: %4d L"),volumenAgua-totalPulsosAguaDia);
      display.setCursor(35,40);
      display.printf(F("Mes: %4d m3"),(volumenAgua-totalPulsosAguaMes)/1000);
      display.setCursor(35,53);
      display.printf(F("Tot: %4d m3"),volumenAgua/1000);
      drawbar(41,60,numPag);
     break;
     case 3:                                                                //Página Test muestra flujo y totalizador temporal para verificar la calibración del flujometro
      display.setDrawColor(1);
      neopixel.setPixelColor(0,0);
      display.drawXBMP(3, 20,25,31,flujo_test_bmp); 
      display.setFont(font1);
      display.setCursor(35,23);
      display.print(flujoAgua,1);
      display.print(F(" LPM"));
      display.setCursor(35,48);
      display.print(float(totalPulsosAgua-totalPulsosCal)/360);
      display.print(F(" L"));
      drawbar(41,60,numPag);
     break;
     case 4:                                                                //Página muestra presión de agua 
      display.setDrawColor(1);
      neopixel.setPixelColor(0,0);
      display.drawXBMP(8, 20,17,22,pres_bmp);  
      display.setCursor(37,25);
      display.print(presAgua,2);
      display.print(F(" PSI"));
      display.setCursor(43,48);
      display.printf(F("%3d %%"),porcPresAgua);
      drawbar(41,60,numPag);
     break;
     case 5:                                                                //Página muestra la hora y fecha
      display.setDrawColor(1);
      neopixel.setPixelColor(0,0);
      display.drawXBMP(7, 10,16,16,reloj_bmp);  
      display.setFont(font2);
      display.setCursor(35,20);
      display.printf( F("%02d:%02d:%02d"), now.hour(),now.minute(),now.second());
      display.setCursor(25,45);
      display.printf( F("%02d/%02d/%04d"), now.day(),now.month(),now.year());
     drawbar(41,60,numPag);
     break;
     case 6:                                                                //Página sin contenido (apaga la pantalla)
     break;
  }
  } while ( display.nextPage() );
  neopixel.show();
}

void setup() {
  pinMode(flowPin, INPUT);
  pinMode(selButtonPin, INPUT);
  pinMode(termaButtonPin, INPUT);
  pinMode(termaOnPin, INPUT);
  pinMode(termaResPin,INPUT);
  pinMode(pirPin, INPUT);
  pinMode(buzPin, OUTPUT);
  pinMode(ledTxPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(flowPin, HIGH);
  digitalWrite(selButtonPin, HIGH);
  digitalWrite(termaButtonPin, HIGH);
  digitalWrite(termaOnPin, HIGH);
  digitalWrite(termaResPin, HIGH);
  attachInterrupt(1, flow, RISING);
  digitalWrite(buzPin,LOW); 
  display.begin();   
  rtc.begin();         
  neopixel.begin();
  neopixel.setBrightness(50);
  dht.setup(DHTPin);
  DS18b20.begin(); 
  DS18b20.setResolution(9);
  attachPinChangeInterrupt(selButtonPin,changepage,FALLING);
  attachPinChangeInterrupt(termaButtonPin,localmode,RISING);
  attachPinChangeInterrupt(termaOnPin,stTerma,CHANGE);
  attachPinChangeInterrupt(pirPin,pir,CHANGE);
  interrupts();  
  
  cloopTime1,cloopTime2,cloopTime3,cloopTime4 = millis();
  now = rtc.now();
  termaTimeOn=now.unixtime();
  rtc.readnvram((uint8_t*)&totalPulsosAgua, 4, 0);                           //Lee dato de volume de agua total de la NVRAM del DS1107
  rtc.readnvram((uint8_t*)&totalPulsosAguaDia, 4, 4);                        //Lee dato de volume de agua dia de la NVRAM del DS1107
  rtc.readnvram((uint8_t*)&totalPulsosAguaMes, 4, 8);                        //Lee dato de volume de agua mes de la NVRAM del DS1107
  rtc.readnvram((uint8_t*)&setTempMax, 1, 12);                               //Lee dato de seteo de temperatura maxima de la NVRAM del DS1107
  rtc.readnvram((uint8_t*)&startcounter, 2, 13);                             //Lee dato de veces iniciado el dispositivo
  startcounter++;
  rtc.writenvram(13, (uint8_t*)&startcounter, 2);                            //Incrementa contador de inicios

  fechaDia=now.day();
  fechaMes=now.month();
  for (int i=0;i<5;i++){                                                     //Parpadeo led en blanco cuando inicia el dispositivo
        neopixel.setPixelColor(0,neopixel.Color(128,128,128));
        neopixel.show();
        delay(500);
        neopixel.setPixelColor(0,neopixel.Color(0,0,0));
        neopixel.show();
        delay(500);
    }
}

void loop() {
  wdt_enable(WDTO_500MS);
  if (!settled && millis() > FILTERSETTLETIME) {
    settled = true;
    rf69_initialize(nodeId, RF_freq, netGroup);
  }
  
  buzzerbip ();
  lowpassFilter.input(analogRead(presPin));                                 //Filtrando ruido mediante el metodo de calculo de promedio de la entrada analoga
  lowpassFilter.input(lowpassFilter.output());                              //Doble filtrado de señal de presion de agua
  
  if (rf69_recvDone()){
     if(rf69_hdr==(RF69_HDR_CTL|RF69_HDR_DST|nodeId )) ackRecv=1;           //Recibe el paquete ACK para verificar que la transmisión fue completada
     if(rf69_receiveStart(nodeIdEmonPi,&emonrx,sizeof(emonrx))){            //Recibe datos enviados del EmonPi desde NodeRed
      switch(emonrx.dataPos){
      case 1:
        setTempMax=emonrx.setTempMax;                                       //Seteo de la temperatura maxima cuando el control se realiza desde NodeRed
        rtc.writenvram(12, (uint8_t*)&setTempMax, 1);
        break;
      case 2:
        totalPulsosAgua=emonrx.setVolumenAgua*360;                          //Ajusta el volumen de agua consumido con los datos recibidos desde Node-Red
        break;
      case 3:
        rtc.adjust(DateTime(emonrx.setUnixTime));                           //Ajusta la fecha y hora del reloj con los datos recibidos desde Node-Red
        numPag=5;
        break;
      }
    }
  }
  currentTime = millis();

  if(currentTime-cloopTime1 >= 350 && buzBip==0)                            //Refresco de pantalla cada 350ms
  {
    cloopTime1 = currentTime;
    loadpage(numPag);
  }
  

 if(currentTime-cloopTime2 >= 1000)                                         //Lectura de sensores de presión y flujo de agua cada 1 segundo
  { 
    lastStTerma=!digitalRead(termaOnPin);
    StResTerma=!digitalRead(termaResPin);
    if (millis() - lastChangeTime[1] > 2000) termaBotonOn=false;
    cloopTime2 = currentTime;
    flujoAgua = (flujoPulsos/6);                                            //Factor de calibración 6 método balde 16L error 1%
    flujoPulsos = 0;                                                    
    volumenAgua=totalPulsosAgua/360;                                        //Volumen de agua consumido en litros                                                                                     
    presAgua=max(0,min(8.7,(8.7/1024*lowpassFilter.output())));             //Sensor de presión ZHIPU Rango: 0-8.7PSI  Salida: 0-5V  (0-60kPa)
    porcPresAgua=presAgua/presInMax*100;                                    //Calculo de la presion de agua en porcentaje en relacion a la presión máxima de ingreso
    now = rtc.now();
    rtc.writenvram(0, (uint8_t*)&totalPulsosAgua, 4);                       //Guarda consumo de agua en la NVRAM del DS1107
    
    if(resetTot && !digitalRead(selButtonPin)){                             //Resetea totalizadores si se mantiene presionado el pulsador select por mas de 8 segundos
      timeCountReset++;
      if(timeCountReset>8) {
        numPag=2;
        totalPulsosAgua=0;
        totalPulsosAguaDia=0;
        totalPulsosAguaMes=0;
        startcounter=0;
        rtc.writenvram(4, (uint8_t*)&totalPulsosAguaDia, 4);
        rtc.writenvram(8, (uint8_t*)&totalPulsosAguaMes, 4);
        rtc.writenvram(13, (uint8_t*)&startcounter, 2); 
      }
    }
    else {
      resetTot=false;
      timeCountReset=0;
    }
    
    if(numPag!=3 && pirTimeOn>screenSaverTime) numPag=6;                     //Apaga el display despues de un tiempo definido y se regresa a la página principal, en la página test se deshabilita el apagado de display 
    else pirTimeOn++;
    if (settled) sendData();                                                 //Transmisión de datos por RF cada 1 segundo
    //digitalWrite(ledPin,(txErr>10 ?1:0));                                  //Enciende el led para indicar que ha habido mas de 10 errores de tx durante el ultimo minuto
  }

  if(currentTime-cloopTime3 >= 5000)                                         //Lectura de sensores de temperatura de la terma y temperatura y humedad del ambiente cada 5 segundos
  {
    cloopTime3 = currentTime;
    DS18b20.requestTemperatures();
    tempTerma=DS18b20.getTempCByIndex(0);    
    tempAmb=dht.getTemperature();
    tempAmb=isnan(tempAmb)?-127 :tempAmb;
    humAmb=dht.getHumidity();
    humAmb=isnan(humAmb)?-127 :humAmb;
    porcTempTerma=(tempTerma!=-127 && tempAmb!=-127) ?min(max(0,((tempTerma-tempAmb)/(setTempMax-tempAmb)*100)),100) :-127;
  }

  if(currentTime-cloopTime4 >= 60000)                                        //Revisa si se ha cambiado de dia o mes para actualizar los totalizadores cada 1 minuto
  {
    cloopTime4 = currentTime;
    if(fechaDia!=now.day()){
      totalPulsosAguaDia=volumenAgua;
      fechaDia=now.day();
      rtc.writenvram(4, (uint8_t*)&totalPulsosAguaDia, 4);
    }
    if(fechaMes!=now.month()){
      totalPulsosAguaMes=volumenAgua;
      fechaMes=now.month();
      rtc.writenvram(8, (uint8_t*)&totalPulsosAguaMes, 4);
    }
  }
  wdt_reset();
}