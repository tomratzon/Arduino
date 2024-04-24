// UART

//global vars
unsigned long clk1=0,clk2=0;
int curr_time_tx;
int curr_time_rx;

int sent_bit=0,rec_bit=0; // # of bot sent/recieved
int num_of_samples=0;
 

// For transmitter data
int data=0b10000001;
int new_data=data;
int new_data_C=new_data;
int data_bit;
int parity_bit=0;
int offset_count=3;

// For reciever data
int sampled_bits=0;
int saved_data=0;
int saved_bit=0;
int rx_parity=0;
int new_bit=0;
int bits_to_ignore=0; //in case of frame drop
int count=3;

void setup()
{
  Serial.begin(9600);
  pinMode(2, INPUT); // pin 2 - Data Input
  pinMode(3, OUTPUT); // pin 3 - Data Output 
 
  digitalWrite(3,HIGH);
  #define BIT_TIME 2000 // 1 Period
  #define SAMP_NUM 3 // 3 samples per bit

}

void loop()
{
  uart_tx();
  uart_rx();
}

void uart_tx()
{
  // Send Data - Always
  //sending order :0(start),d7,d6,d5,d4,d3,d2,d1,d0,PB,1(stop)
  if(clk1_check_tx()==0)
    return; // return until one cycle completes
  
  if(sent_bit==0) //sending start bit
  {
    digitalWrite(3,LOW); //set pin 3 to low
    sent_bit++;
    Serial.print("Sent start bit (0)\n");
    return;
  }
  if(sent_bit==9)//sending parity bit
  {
    parity_bit=parity_bit%2;
    parity_bit=!parity_bit;
    digitalWrite(3,parity_bit);
    Serial.print("Sent parity bit: ");
    Serial.print(parity_bit);
    Serial.print("\n");
    sent_bit++;
    return;
  }
  if(sent_bit==10) //end meddage and randomize new message to send
  {
    digitalWrite(3,HIGH); //set pin 7 to High
    Serial.print("Sent stop bit (1)\n");
    parity_bit=0;
    Serial.print("Sent Message: ");
    Serial.print(new_data);
    Serial.print("\n");
    new_data=random(0,255); //randomize new 8 bit message
    new_data_C=new_data;
    sent_bit=0;
    return;
    
  }
  else //send one of the 8 bit of the data
  {
  	data_bit=new_data_C & 0b00000001; //extract lsb bit from the 8 bit message
  	new_data_C=new_data_C>>1;
  	digitalWrite(3,data_bit); //set pin 3 to the extracted lsb bit
  	parity_bit+=data_bit; //sum the bit into the parity bit
    Serial.print("Sent Data Bit: ");
    Serial.print(data_bit);
    Serial.print("\n");
  	sent_bit++;
  	return;  
  }
}

void uart_rx(){
  
  if(clk2_check_rx()==0) // return until sample time arrived (period/# samples)
    return; 
  //sampling...
  else
  {
    if(offset_count!=0)
  	{
    	offset_count--;
      	//Serial.print("offset_count: ");
       // Serial.print(offset_count); 
        //Serial.print("\n"); 
    	return;
  	}
  }
  
  
   //if(bits_to_ignore!=0) //incase of previous frame drop, ignoring the rest of the bits from the drop frame
   //{
    // Serial.print("there are bits to ignore\n"); 
    // bits_to_ignore--;
    // rec_bit=0;
    // return;
   //}
  saved_bit=sample_bits();
  if(saved_bit==2)
    return; // unfinished sampling
  //if(saved_bit==3)
  //{
   // bits_to_ignore=10-rec_bit; 
  //  Serial.print("waiting for new message...\n");
  //	return;
//}
  if(rec_bit==0)
    {
      rec_bit++;
      Serial.print("Recieved start bit: ");
   	  Serial.print(saved_bit);
      Serial.print("\n");
      return;
    }
   if(rec_bit==9)
     //checking parity bit
   {
      Serial.print("calculated parity bit: ");
      Serial.print(!rx_parity);
      Serial.print("\n");
     
      Serial.print("print saved PB bit: ");
      Serial.print(saved_bit);
      Serial.print("\n");
      rx_parity=rx_parity%2;//odd or even number of 1's in the data (0 d=for even, 1 for odd)
     //the next line can be explained as such:
     //if the rx_parity is 1 then received parity bit should be 0
     //if the rx_parity is 0 then received paruty bit should be 1
     if(rx_parity==saved_bit)
     	Serial.print(" Error, Parity Bit Not Valid! \n");  
     rec_bit++;
     return;    
   }
   if(rec_bit==10)
   {
     if(saved_bit==0)
       return; //wrong stop bit
      rec_bit=0;
      Serial.print("Recieved Message: ");
      Serial.print(saved_data);
      Serial.print("\n");
      rx_parity=0; 
      saved_data=0;
      return;
   }
 
  else
  {
  	rx_parity+saved_bit; //add received bit to parity
  	Serial.print("Received Data Bit:");
    Serial.print(saved_bit);
    Serial.print("\n");
 	saved_bit=saved_bit<<(rec_bit-1); //rotate recieived bit left to correct place(-1 for start the parity)
  	saved_data=saved_data+saved_bit; //add received bit to data
  	rec_bit++;
  	return;
  }
}

int sample_bits(){
  // this function will sample from signal from pin 2
  // after SAMP_NUM(=3) will return sampled signal if all bits are the same in sampled_bits
  // returns 2 if samples are not finished for the current period
  // returns 0 for finished, valid sample with the value 0
  // returns 1 for finished, valid sample with the value 1
  // returns 3 for corrupt sample
  
  new_bit = digitalRead(2); //read from pin 2
  
  //Serial.print("sample number: |");
  //Serial.print(count);
  //Serial.print("|");
  //count--;
  //Serial.print("this read bit: ");
  //Serial.print(new_bit);
  //Serial.print("\n");
  //if(count==0)
   // count=3;
 
 
 
  sampled_bits += new_bit;
  sampled_bits = sampled_bits << 1; //rotate 1 bit to the left
  num_of_samples++;
  
  
  if(num_of_samples!=SAMP_NUM) 
    return 2;
  if((sampled_bits & 0b00001110) == 0b00000000) //sampled_bits are = 000
  {
    
    sampled_bits=0;
    num_of_samples=0;
    return 0;
  }
  if((sampled_bits & 0b00001110) == 0b00001110) //sampled_bits are = 000
  { 
    
    sampled_bits=0;
    num_of_samples=0;
    return 1;
  }
  else
  {
    //Serial.print("Error, corrupt sample, droping frame...\n");
  	num_of_samples=0;
  	return 3;
  }
}

int clk1_check_tx(){
  //the transmiter checks when a full period has passed 
  //then he can send the next bit
  curr_time_tx=millis();
  if(curr_time_tx-clk1>BIT_TIME+20)
  {
    clk1=curr_time_tx;
    return 1;
    
  }
  return 0;
}

int clk2_check_rx(){
  // the receiver checks when a (period time/# samples) has passed 
  // the he can take the next sample
  curr_time_rx=millis();
  if(curr_time_rx-clk2>BIT_TIME/SAMP_NUM)
  {
    clk2=curr_time_rx;
    return 1;
  }
  return 0;
}  