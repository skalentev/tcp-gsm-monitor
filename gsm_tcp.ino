
/* GSM Shield example
 
 created 2011
 by Boris Landoni
 
 This example code is in the public domain.

 
 http://www.open-electronics.org
 http://www.futurashop.it
 */

#include <SoftwareSerial.h> 
#include <GSM_Shield.h>
#include <OneWire.h>

//**************************************************************************
char number[]="+79279767758";  //Destination number 
char text[]="hello world";  //SMS to send
byte type_sms=SMS_UNREAD;      //Type of SMS
byte del_sms=0;                //0: No deleting sms - 1: Deleting SMS
//**************************************************************************

GSM gsm;
char sms_rx[122]; //Received text SMS
//int inByte=0;    //Number of byte received on serial port
char number_incoming[20];
int call;
int error;
OneWire  ds(13);  // on pin 13 (a 4.7K resistor is necessary)
byte ds_pwr = 12;
static uint32_t timer;
static char buff[100];
static byte state = 0;  
static double celsius;
static  int16_t raw;
 static byte present = 0;
static  byte addr[8];

void setup() 
{
  pinMode(ds_pwr, OUTPUT); 
  Serial.begin(9600);
  Serial.println("system startup"); 
  digitalWrite(ds_pwr, HIGH); 

}


void loop()
{ 
  char inSerial[50];   
  int i=0;
  delay(2000);
  
 //   Check_Call(); //Check if there is an incoming call
 //   Check_SMS();  //Check if there is SMS 
    //Check data serial com 
    
    if (Serial.available() > 0) 
    {             
       while (Serial.available() > 0) {
         inSerial[i]=(Serial.read()); //read data  
         i++;      
       }
       inSerial[i-1]='\0';
      Check_Protocol(inSerial);
    }
       
       
       
       
 if (millis() > timer ){
    timer = millis() + 30000;
  byte i;
  byte type_s;
  byte data[12];
  byte cfg;
   
  Serial.print("stade:");
  Serial.println(state);
   switch (state){
     case 0:
         if ( !ds.search(addr)) {
           Serial.println("No more addresses.");
            ds.reset_search();
            timer = millis()+250;
            state = 0;
          } else {
            timer = millis()+50;
            state = 1;
          }
     break;
     case 1:
     timer = millis()+50;
            state = 2;
     break;
     case 2:
         ds.reset();
         ds.select(addr);
         ds.write(0x44, 1);  
      state = 3;//!!!!!!!!!!
      timer = millis()+1000;
      break;
     case 3:
          present = ds.reset();
          ds.select(addr);    
          ds.write(0xBE);         // Read Scratchpad
         for ( i = 0; i < 9; i++) {           // we need 9 bytes
            data[i] = ds.read();
          }
              raw = (data[1] << 8) | data[0];
         cfg = (data[4] & 0x60);
              // at lower res, the low bits are undefined, so let's zero them
              if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
              else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
              else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
            celsius = (float)raw / 16.0;
            Serial.println(celsius);
  sprintf(buff,"#6C:E8:73:D2:4D:F6#EtherCard\r\n#%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x#%2d.%02d\r\n##",
              addr[0],addr[1],addr[2],addr[3],addr[4],addr[5],addr[6],addr[7],
              int(celsius),int(celsius*100)%100);
      state = 4;
      timer = millis()+1000;
     break;
     case 4:
        gsm.TurnOn(9600);          //module power on
        gsm.InitParam(PARAM_SET_1);//configure the module  
        gsm.Echo(0);               //enable AT echo 
      state = 5;
      timer = millis()+10000;
     break;
      case 5:
      sendToHost(buff);
 gsm.SendATCmdPrintResp("AT+CPOWD=1", 5000, 1000, 1);
 state = 0;
      timer = millis()+300000;

      break;
    default:
     state = 0;
     timer = millis()+300000;
   }
}

       
       
       
       
       
       
       
}  

static void sendToHost(char* postval){
   Serial.print("Init GPRS");
         error=gsm.InitGPRS();  
         if (error==1)  //Check status
         {
             Serial.println("Init OK \n");
         }
         else
         {
             Serial.println("INIT ERR \n");
             Serial.println(error);             
         }
         
         gsm.SendATCmdPrintResp("AT+CIFSR", 5000, 1000, 1);
         
         Serial.print("Sending MSG");
    //     error=gsm.SendTCP("vps.h-net.ru","8888", postval); 
                  error=gsm.SendTCP("narodmon.ru","8283", postval); 

         if (error==1)  //Check status
         {
             Serial.println("Send OK \n");
         }
         else
         {
             Serial.println("Send ERR \n");
             Serial.println(error);             
         }

}


void Check_Protocol(String inStr)
{   
       Serial.print("Command: ");
       Serial.println(inStr);
       
  Serial.println("Check_Protocol");
  
    switch (inStr[0])
      {
         
       case 's': //S //Send SMS
        sendToHost("Hello Everybodey! sending message!!!!!!!!!!!!!!!!!");
         
         break;
         default:
         char buf[100];
         inStr.toCharArray(buf,100);
         gsm.SendATCmdPrintResp(buf, 5000, 1000, 1);
              
         
       }
   
    delay(1500);
    
    return;
 }
 
 
 void Check_Call()  //Check status call if this is available
 {     
     call=gsm.CallStatus();
     switch (call)
     {    
       case CALL_NONE:
         Serial.println("no call");
         break;
       case CALL_INCOM_VOICE:
         gsm.CallStatusWithAuth(number_incoming,0,0);        
         Serial.print("incoming voice call from ");     
         Serial.println(number_incoming);
         break;
       case CALL_ACTIVE_VOICE:
         Serial.println("active voice call");    
         break;
       case CALL_NO_RESPONSE:
         Serial.println("no response");
         break;
     }
     return;
 }
 
 
 void Check_SMS()  //Check if there is an sms 'type_sms'
 {
     char pos_sms_rx;  //Received SMS position     
     pos_sms_rx=gsm.IsSMSPresent(type_sms);
     if (pos_sms_rx!=0)
     {
       //Read text/number/position of sms
       gsm.GetSMS(pos_sms_rx,number_incoming,sms_rx,120);
       Serial.print("Received SMS from ");
       Serial.print(number_incoming);
       Serial.print("(sim position: ");
       Serial.print(word(pos_sms_rx));
       Serial.println(")");
       Serial.println(sms_rx);
       if (del_sms==1)  //If 'del_sms' is 1, i delete sms 
       {
         error=gsm.DeleteSMS(pos_sms_rx);
         if (error==1)Serial.println("SMS deleted");      
         else Serial.println("SMS not deleted");
       }
     }
     return;
 }
 
 


 

  
