// USART

//global vars
// For clock transmission
unsigned long clock=0;
int edge=0;
int currTime;

int sent_bit=0,rec_bit=0; //# of bit sent/recieved
int data=0b01010111; // initial data
int data_copy=data; //copy of the original data we can manipulate

// Transmitter
int new_data=data;
int data_bit;
int clk_check_tx=0;

// Reciever
int clk_check_rx=0;
int saved_data=0;
int saved_bit=0;

void setup()
{
  Serial.begin(9600);
  pinMode(2, INPUT); // pin #2 - Clock Data Input
  pinMode(3, OUTPUT); // pin #3 - Clock Data Output
  pinMode(4, INPUT); // pin #4 - Data Input
  pinMode(5, OUTPUT); // pin #5 - Data Output 
  #define BIT_TIME 400 // 1 Period
  
    
}

void loop()
{
  usart_tx();
  usart_rx();
}

void usart_tx(){
   // Send Data - Always
  
  clk_trasmit();
  if(clk_check_tx==0 && digitalRead(3)==HIGH){ 
    // Checking for rising edge on pin 3 and last clk check was 0
    //means edge change and we can send bit
    data_bit=data_copy & 0b00000001; //extract lsb from data
    Serial.print("Sent bit: ");
    Serial.print(data_bit);
    Serial.print("\n");
    
    data_copy=data_copy>>1; //rotate right (lsb=next bit after the lsb)
    digitalWrite(5,data_bit); //set pin 5-data output to new sent bit
    sent_bit++;
    clk_check_tx=1; // There was a rising edge => change last clk check to 1
    if(sent_bit==8){
      //if 8 bit were sent do:
      sent_bit=0;
 
      Serial.print("Sent: ");
      Serial.print(new_data);
      Serial.print("\n");
      Serial.print("END OF BYTE\n");
      new_data=random(0,255); //rendomize new 8 bit msg
      data_copy=new_data;
      Serial.print("new msg to send: ");
      Serial.print(new_data);
      Serial.print("\n");
   
      
    }  // and data transfer complete
    return;
  }
  clk_check_tx=digitalRead(3); // Sample clock,can send again at next raising edge
}

void usart_rx(){
  if(clk_check_rx==1 && digitalRead(2)==LOW){ 
    // Checking for falling edge on pin 2 and that last read clk check was 1
    //means edge change and can read sent bit
    saved_bit=digitalRead(4);//read set bit to save_bit
    Serial.print("received bit: ");
    Serial.print(saved_bit);
    Serial.print("\n");
    saved_bit=saved_bit<<rec_bit; //rotate save_bit to the left by # of rec_bit
    saved_data=saved_data+saved_bit; //sum the received bit to saved_date
    rec_bit++;
    clk_check_rx=0; // There was a falling edge
    if(rec_bit==8) {
      Serial.print("Recieved new msg: ");
        Serial.print(saved_data);
        Serial.print("\n");
      saved_data=0;
      rec_bit=0;
    }
    return;
  }
  clk_check_rx=digitalRead(2); // Sample clock, can read again at next fallin edge
}

void clk_trasmit(){
  // works as the clock,every BIT_TIME period change the clock edge
  currTime=millis();
  if(currTime-clock>BIT_TIME){
    clock=currTime;
    edge=!edge;
    digitalWrite(3,edge);//set currwnt edge to pin 3
  }
}  
