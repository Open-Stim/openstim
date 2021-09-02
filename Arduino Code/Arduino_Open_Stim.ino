/* ===============================================================================

           FEDERAL UNIVERSITY OF SÃO JOÃO DEL-REI - UFSJ 
           GRADUATE PROGRAM IN ELECTRICAL ENGINEERING - PPGEL
           LABORATORY OF NEUROENGINEERING AND NEUROSCIENCE – LINNCE
        
             - OPEN STIM - ELECTRICAL STIMULATOR OPEN SOURCE HARDWARE -

   --> DEVELOPED BY: 

        1. MAIKON LORRAN SANTOS
        2. JOÃO DANIEL NOLASCO
        3. VINÍCIUS ROSA COTA

=============================================================================== */

// --- Auxiliary Library ---

#include  "Nextion.h"   //Nextion library

// ===============================================================================
// --- Display Nextion Object Declaration ---
 
NexDSButton bt0   = NexDSButton(1, 11, "bt0");  // Start / Stop
NexDSButton bt1   = NexDSButton(1, 9, "bt1");  // Positive pulse
NexDSButton bt2   = NexDSButton(1, 10, "bt2");  // Negative pulse
NexNumber x0      = NexNumber(1, 7, "x0"); // Frequency: 0.1 - 300Hz
NexNumber x1      = NexNumber(1, 8, "x1"); // Pulse duration: 0.004 - 1000ms

// ===============================================================================
// --- Hardware Mapping ---

byte pino_OCR4D = D0; //6; // The duty cycle does not vary. Base for positive pulse
byte pino_OCR4B = D1; // 10; // Varies work cycle and frequency. Cut the beginning of the wave from pin 6.
byte pino_OCR4D_invertido = D2; // 12; // The duty cycle does not vary. Base for the negative pulse.
byte pino_OCR4A_invertido = D3; // 5; // Varies work cycle and frequency. Cut the end of the wave from pin 12.
byte pino_OCR4B_invertido = D4; // 9; // It is not used but generates PWM
byte pino_OCR4A = D5; // 13; // It is not used but generates PWM


// ================================================================================================================
// --- Global Variables ---

uint32_t ds_ST;     // Stores the start / Stop Status
uint32_t ds_P;      // Stores the status of the Positive button
uint32_t ds_N;      // Stores the state of the Negative button

word contador_periodo = 0; // The counter timer 4 of the Arduino Leonardo has a resolution of 10 bits.
word contador_largura_do_pulso = 0; //The counter timer 4 of the Arduino Leonardo has a resolution of 10 bits.

double frequencia_desejada = 0; // 0.1 Hz to 300 Hz per design choice
double largura_do_pulso_desejada = 0; // 0.0004 ms up to 1000 ms per project choice

uint32_t decimal; // Stores the value entered on the Nextion screen
String recebido; // Convert received number to string
double numero_F; // Convert the received number to Double
double numero_D; // Convert the received number to Double

double limite_largura_pulso; // It stores double values ​​temporarily, to reduce the number of calculations.

// =================================================================================================================

/*=================================================================================================
NOTE 1:
 
  1. Pins 6 and 10 generate the positive pulse
  2. Pins 5 and 12 generate the negative pulse
  3. Arduino connection: pins 5 and 6 on the stage 1 op amp and 10 and 12 on the stage 2 op amp

=================================================================================================== =*/

void setup() // DEFINES INITIAL CONDITION

{
  nexInit();  // Initializes Nextion tft display
  Serial.begin(115200); // Communication rate
 // TCCR4D = 0b00000000; // The value of this register remains unchanged throughout the code.
 Serial.println("Inicializando ... ");
}

void loop()
{
  // INFINITE LOOP START: 

  bt0.getValue(&ds_ST); // Start / Stop button
  bt1.getValue(&ds_P); // Positive pulse button
  bt2.getValue(&ds_N); // Negative pulse button

   
  switch (ds_ST)
  {
    
    case 0:
    digitalWrite(pino_OCR4D,LOW); // Pin 6
    digitalWrite(pino_OCR4B,LOW); // Pin 10
    pinMode(pino_OCR4D,INPUT); // Define pin 6 as input
    pinMode(pino_OCR4B,INPUT); // Define pin 10 as input
    digitalWrite(pino_OCR4D_invertido,LOW); // Pin 12
    digitalWrite(pino_OCR4A_invertido,LOW); // Pin 5
    pinMode(pino_OCR4D_invertido,INPUT); // Defines pin 12 as input
    pinMode(pino_OCR4A_invertido,INPUT); // Defines pin 5 as input
    digitalWrite(pino_OCR4B_invertido,LOW); // Pin 9
    digitalWrite(pino_OCR4A,LOW); // Pin 13
    pinMode(pino_OCR4B_invertido,INPUT); // Define pin 9 as input
    pinMode(pino_OCR4A,INPUT); // Define pin 13 as input
    break;
    
    case 1:
    if (ds_P > 0)
    {
      digitalWrite(pino_OCR4D_invertido,LOW); // Pin 12
      digitalWrite(pino_OCR4A_invertido,LOW); // Pin 5
      pinMode(pino_OCR4D_invertido,OUTPUT); // Defines pin 12 as output
      pinMode(pino_OCR4A_invertido,OUTPUT); // Defines pin 5 as output
    }
    else
    {
      digitalWrite(pino_OCR4D_invertido,LOW); // Pin 12
      digitalWrite(pino_OCR4A_invertido,LOW); // Pin 5
      pinMode(pino_OCR4D_invertido,INPUT); // Defines pin 12 as input
      pinMode(pino_OCR4A_invertido,INPUT); // Defines pin 5 as input
    }
    if (ds_N > 0)
    {
      digitalWrite(pino_OCR4D,LOW); // Pin 6  
      digitalWrite(pino_OCR4B,LOW); // Pin 10   
      pinMode(pino_OCR4D,OUTPUT); // Defines pin 6 as output
      pinMode(pino_OCR4B,OUTPUT); // Defines pin 10 as output   
    }
    else
    {
      digitalWrite(pino_OCR4D,LOW); // Pin 6
      digitalWrite(pino_OCR4B,LOW); // Pin 10   
      pinMode(pino_OCR4D,INPUT); // Define pin 6 as input
      pinMode(pino_OCR4B,INPUT); // Define pin 10 as input
    }
    break;
  }
  
  // Reads the entered frequency directly from the Nextion screen:
  
  x0.getValue(&decimal); // x0 stores the frequency value on the Nextion screen
  recebido = String(decimal); // Converts the value to string
  numero_F = recebido.toDouble(); // Converts the value to Double
  frequencia_desejada = numero_F / 10; // Value already in decimal
  if (frequencia_desejada < 0.1) // 0.1 Hz is the lower frequency limit chosen in the project.
  {
    frequencia_desejada = 0.1; // Project limitation
    x0.setValue(1);
  }

 


  // Read directly from the Nextion screen the pulse duration entered:
  
  x1.getValue(&decimal); // x1 stores the pulse width value on the Nextion screen
  recebido = String(decimal); // Converts the value to string
  numero_D = recebido.toDouble(); // Converts the value to Double
  largura_do_pulso_desejada = numero_D / 1000; // Go to milliseconds.
  if (largura_do_pulso_desejada < 0.004) // The minimum value chosen in the design for the pulse width is 0.004 ms.
  {
    largura_do_pulso_desejada = 0.004; // Project limitation
    x1.setValue(4);
  }
  
 // largura_do_pulso_desejada = largura_do_pulso_desejada/1000; // Go from milliseconds to seconds
  limite_largura_pulso = ((1/frequencia_desejada)/2);  // The highest possible value for the pulse width is 50% because the wave generated must be symmetrical. Value in seconds.
 
  //    == END OF BASIC CALCULATIONS ==   //

 /* ESP8266 Implementation */


 /* analogWriteFreq(frequencia_desejada);
 analogWriteRange(1023);
 analogWrite(0, (int)(1023*largura_do_pulso_desejada*frequencia_desejada)); // Largura do pulso = 1 */

  verificaPWM(frequencia_desejada, largura_do_pulso_desejada);
}
  
void verificaPWM(double frequencia_desejada, double largura_do_pulso_desejada){


  static bool level = false;
  static long last_millis_pulso=millis(), last_millis_comprimento=millis();
  double comprimento_onda =  1.0 / frequencia_desejada;
  
  if (level && ((millis() - last_millis_pulso) > largura_do_pulso_desejada*1000)) {
      level = false;
  } else if (!level && ((millis() - last_millis_comprimento) > comprimento_onda*1000) && (largura_do_pulso_desejada > 0.0)){
    level = true;
    last_millis_comprimento = millis();
    last_millis_pulso = millis();
  }
  if (level){
    digitalWrite(pino_OCR4D,HIGH);
    digitalWrite(pino_OCR4D_invertido,LOW);
  } else {
    digitalWrite(pino_OCR4D,LOW);
    digitalWrite(pino_OCR4D_invertido,HIGH);
  }
  
}

/*========================================================================================================================================================
NOTE 2:
   With the frequency CLOCK_PRINCIPAL equal to 16 MHz, maximum prescaler value of timer / counter 4 equal to 16384 and PWM resolution of 1023,
   the minimum possible frequency to be generated is ((16000000/16384) / 1023) = 0.96 Hz.
   Thus, in order to generate frequency waves below 0.96 Hz, the delay () and delayMicroseconds () functions need to be used.
   0.06 Hz would be the value reached, if it were possible to divide the clock frequency from the PLL circuit,  which is at its minimum value of 16 MHz,
   at a value less than or equal to 0.1 Hz (limit of the original project).
   That is, if the timer / counter prescaler 4 reached the value of 262144.

 ========================================================================================================================================================= =*/
 /*
  // FREQUENCY BETWEEN 0.1 HZ AND 0.96 HZ:
   
  if ((frequencia_desejada >= 0.1)&&(frequencia_desejada < 0.96))
  {
    TCCR4E = 0b10000000;
    TCCR4A = 0b00000000; // Disabling PWM mode for pins OC4A, OC4A inverted, OC4B and OC4B inverted.
    TCCR4C = 0b00000000; // Disabling PWM mode for OC4D and OC4D inverted pins.
    TCCR4E = 0b00000000;
    digitalWrite(pino_OCR4D,HIGH);
    digitalWrite(pino_OCR4D_invertido,LOW);
    if ((largura_do_pulso_desejada*1000) < 1) // If the desired pulse_width is less than 1 ms, it is necessary to use the delayMicroseconds () function.
    {
      delayMicroseconds(largura_do_pulso_desejada*1000000);
    }
    else
    {
      delay(largura_do_pulso_desejada*1000);
    }
    digitalWrite(pino_OCR4D,LOW);
    digitalWrite(pino_OCR4D_invertido,HIGH);
     if ((largura_do_pulso_desejada*1000) < 1) // If the desired pulse_width is less than 1 ms, it is necessary to use the delayMicroseconds () function.
    {
      delayMicroseconds(largura_do_pulso_desejada*1000000);
    }
    else
    {
      delay(largura_do_pulso_desejada*1000);
    }
    digitalWrite(pino_OCR4D_invertido,LOW);
     if ((largura_do_pulso_desejada*1000) < 1)
    {
      delayMicroseconds((0.002-(2*largura_do_pulso_desejada))*1000000); // Accuracy improvement
      delay(((1.0/frequencia_desejada)-0.002)*1000);
    }
    else
    {
       delay(((1.0/frequencia_desejada)-(2*largura_do_pulso_desejada))*1000);
    }
  }

/*========================================================================================================================================
NOTE 3:
   The value of the period_counter is calculated by dividing the frequency of the system clock source by the system clock divider (= 1), 
   then by the timer / counter divider 4 and, finally, by the desired frequency value.
   The same formula is used to calculate the lower limit of the desired_frequency on each value of the system clock divider.
   In the following if statements, the calculated lower limit of the desired_frequency is always rounded up. Thus, 
   the counter_period does not exceed its upper limit (1023)

 ========================================================================================================================================= =* /
  
  // FREQUENCY BETWEEN 0.96 HZ AND 1.91 HZ:
  
  if ((frequencia_desejada >= 0.96)&&(frequencia_desejada < 1.91))
  {
    TCCR4E = 0b10000000; // Prevents the calculated values ​​from being written to the comparison registers, that is, the values ​​are stored in buffers. Subsequently, all registrars will be simultaneously written.
    TCCR4A = 0b01010011; // Enabling PWM mode for pins OC4A, OC4A inverted, OC4B and OC4B inverted. OC4A and OC4B change to logic level 0 when the TCNT4 counter equals OCR4A and OCR4B, respectively. These pins change to logic level 1 when TCNT4 = 0. Inverted OC4A and inverted OC4B are 180 degrees out of OC4A and OC4B, respectively.
    TCCR4B = 0b00001111; // Defining the factor that divides the 16000000 MHz, that is, the timer / counter prescaler value 4. In this case, 16384.
    TCCR4C = 0b01010101; // Enabling PWM mode for OC4D and OC4D inverted pins. OC4D changes to logic level 0 when the TCNT4 counter equals OCR4D. This pin changes to logic level 1 when TCNT4 = 0. Inverted OC4D is offset by 180 degrees from OC4D. The 4 most significant bits of TCCR4D are the shadow bits of the 4 most significant bits of TCCR4A.
    contador_periodo = round((16000000/16384)/frequencia_desejada);
    // To write values ​​to the 10-bit registers OCR4A, OCR4B, OCR4C and OCR4D, first the 2 most significant bits are sent to TC4H and then the least significant 8 bits are sent to the respective registers.
    // When the least significant 8 bits are sent, the value written in TC4H plus those 8 bits are written to the respective register, in the same clock cycle.
    TC4H = contador_periodo >> 8;
    OCR4C = contador_periodo;
    contador_largura_do_pulso = round(contador_periodo/2);
    TC4H = contador_largura_do_pulso >> 8;
    OCR4D = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)-round(largura_do_pulso_desejada*(16000000/16384)); // Calculation of the cutoff point of the first half cycle.
    if (contador_largura_do_pulso == round(contador_periodo/2))
    {
      contador_largura_do_pulso = contador_largura_do_pulso - 1; // Prevents the pulse width of the first half cycle from being null due to the rounding of the expression (desired_pulse_width * (16000000 / 16384)) to zero.
    }
    TC4H = contador_largura_do_pulso >> 8;
    OCR4B = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)+round(largura_do_pulso_desejada*(16000000/16384)); // Calculation of the cutoff point of the second half cycle.
    if (contador_largura_do_pulso == round(contador_periodo/2))
    {
      contador_largura_do_pulso = contador_largura_do_pulso + 1; // Prevents the pulse width of the second half cycle from being zero due to the rounding of the expression (desired_pulse_width * (16000000/1 6384)) to zero
    }
    TC4H = contador_largura_do_pulso >> 8;
    OCR4A = contador_largura_do_pulso;
    TCCR4E = 0b00000000; // Allows the updating of the comparison registers with the new calculated values, at the end of the current PWM cycle.
  }

/*========================================================================================================================
NOTE 4:
From now on, the same reasoning follows, changing only the prescaler, and therefore, we will not repeat the same comments

==========================================================================================================================* /
  
  // FREQUENCY BETWEEN 1.91 HZ AND 3.32 HZ:
  
  if ((frequencia_desejada >= 1.91)&&(frequencia_desejada < 3.82))
  {
    TCCR4E = 0b10000000;
    TCCR4A = 0b01010011;
    TCCR4B = 0b00001110; // In this case, timer / counter prescaler 4 is set to 8192.
    TCCR4C = 0b01010101;
    contador_periodo = round((16000000/8192)/frequencia_desejada);
    TC4H = contador_periodo >> 8;
    OCR4C = contador_periodo;
    contador_largura_do_pulso = round(contador_periodo/2);
    TC4H = contador_largura_do_pulso >> 8;
    OCR4D = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)-round(largura_do_pulso_desejada*(16000000/8192));
    if (contador_largura_do_pulso == round(contador_periodo/2))
    {
      contador_largura_do_pulso = contador_largura_do_pulso - 1;
    }
    TC4H = contador_largura_do_pulso >> 8;
    OCR4B = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)+round(largura_do_pulso_desejada*(16000000/8192));
    if (contador_largura_do_pulso == round(contador_periodo/2))
    {
      contador_largura_do_pulso = contador_largura_do_pulso + 1;
    }
    TC4H = contador_largura_do_pulso >> 8;
    OCR4A = contador_largura_do_pulso;
    TCCR4E = 0b00000000;
  }

   // FREQUENCY BETWEEN 3.82 HZ AND 7.64 HZ:
   
  if ((frequencia_desejada >= 3.82)&&(frequencia_desejada < 7.64))
  {
    TCCR4E = 0b10000000;
    TCCR4A = 0b01010011;
    TCCR4B = 0b00001101; // In this case, timer / counter prescaler 4 is set to 4096.
    TCCR4C = 0b01010101;
    contador_periodo = round((16000000/4096)/frequencia_desejada);
    TC4H = contador_periodo >> 8;
    OCR4C = contador_periodo;
    contador_largura_do_pulso = round(contador_periodo/2);
    TC4H = contador_largura_do_pulso >> 8;
    OCR4D = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)-round(largura_do_pulso_desejada*(16000000/4096));
    if (contador_largura_do_pulso == round(contador_periodo/2))
    {
      contador_largura_do_pulso = contador_largura_do_pulso - 1;
    }
    TC4H = contador_largura_do_pulso >> 8;
    OCR4B = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)+round(largura_do_pulso_desejada*(16000000/4096));
    if (contador_largura_do_pulso == round(contador_periodo/2))
    {
      contador_largura_do_pulso = contador_largura_do_pulso + 1;
    }
    TC4H = contador_largura_do_pulso >> 8;
    OCR4A = contador_largura_do_pulso;
    TCCR4E = 0b00000000;
  }

   // FREQUENCY BETWEEN 7.64 HZ AND 15.28 HZ:
   
  if ((frequencia_desejada >= 7.64)&&(frequencia_desejada < 15.28))
  {
    TCCR4E = 0b10000000;
    TCCR4A = 0b01010011;
    TCCR4B = 0b00001100; // In this case, timer / counter prescaler 4 is set to 2048.
    TCCR4C = 0b01010101;
    contador_periodo = round((16000000/2048)/frequencia_desejada);
    TC4H = contador_periodo >> 8;
    OCR4C = contador_periodo;
    contador_largura_do_pulso = round(contador_periodo/2);
    TC4H = contador_largura_do_pulso >> 8;
    OCR4D = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)-round(largura_do_pulso_desejada*(16000000/2048));
    if (contador_largura_do_pulso == round(contador_periodo/2))
    {
      contador_largura_do_pulso = contador_largura_do_pulso - 1;
    }
    TC4H = contador_largura_do_pulso >> 8;
    OCR4B = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)+round(largura_do_pulso_desejada*(16000000/2048));
    if (contador_largura_do_pulso == round(contador_periodo/2))
    {
      contador_largura_do_pulso = contador_largura_do_pulso + 1;
    }
    TC4H = contador_largura_do_pulso >> 8;
    OCR4A = contador_largura_do_pulso;
    TCCR4E = 0b00000000;
  }

   // FREQUENCY BETWEEN 15.28 HZ AND 30.55 HZ:
   
  if ((frequencia_desejada >= 15.28)&&(frequencia_desejada < 30.55))
  {
    TCCR4E = 0b10000000;
    TCCR4A = 0b01010011;
    TCCR4B = 0b00001011; // In this case, timer / counter prescaler 4 is set to 1024.
    TCCR4C = 0b01010101;
    contador_periodo = round((16000000/1024)/frequencia_desejada);
    TC4H = contador_periodo >> 8;
    OCR4C = contador_periodo;
    contador_largura_do_pulso = round(contador_periodo/2);
    TC4H = contador_largura_do_pulso >> 8;
    OCR4D = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)-round(largura_do_pulso_desejada*(16000000/1024));
    if (contador_largura_do_pulso == round(contador_periodo/2))
    {
      contador_largura_do_pulso = contador_largura_do_pulso - 1; 
    }
    TC4H = contador_largura_do_pulso >> 8;
    OCR4B = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)+round(largura_do_pulso_desejada*(16000000/1024)); 
    if (contador_largura_do_pulso == round(contador_periodo/2))
    {
      contador_largura_do_pulso = contador_largura_do_pulso + 1; 
    }
    TC4H = contador_largura_do_pulso >> 8;
    OCR4A = contador_largura_do_pulso;
    TCCR4E = 0b00000000; 
  }

   // FREQUENCY BETWEEN 30.55 HZ AND 61.1 HZ:
   
  if ((frequencia_desejada >= 30.55)&&(frequencia_desejada < 61.1))
  {
    TCCR4E = 0b10000000;
    TCCR4A = 0b01010011;
    TCCR4B = 0b00001010; // In this case, timer / counter prescaler 4 is set to 512.
    TCCR4C = 0b01010101;
    contador_periodo = round((16000000/512)/frequencia_desejada);
    TC4H = contador_periodo >> 8;
    OCR4C = contador_periodo;
    contador_largura_do_pulso = round(contador_periodo/2);
    TC4H = contador_largura_do_pulso >> 8;
    OCR4D = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)-round(largura_do_pulso_desejada*(16000000/512));
    if (contador_largura_do_pulso == round(contador_periodo/2))
    {
      contador_largura_do_pulso = contador_largura_do_pulso - 1;
    }
    TC4H = contador_largura_do_pulso >> 8;
    OCR4B = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)+round(largura_do_pulso_desejada*(16000000/512));
    if (contador_largura_do_pulso == round(contador_periodo/2))
    {
      contador_largura_do_pulso = contador_largura_do_pulso + 1;
    }
    TC4H = contador_largura_do_pulso >> 8;
    OCR4A = contador_largura_do_pulso;
    TCCR4E = 0b00000000;
  }

   // FREQUENCY BETWEEN 61.1 HZ AND 122.19 HZ:
   
  if ((frequencia_desejada >= 61.1)&&(frequencia_desejada < 122.19))
  {
    TCCR4E = 0b10000000;
    TCCR4A = 0b01010011;
    TCCR4B = 0b00001001; // In this case, timer / counter prescaler 4 is set to 256.
    TCCR4C = 0b01010101;
    contador_periodo = round((16000000/256)/frequencia_desejada);
    TC4H = contador_periodo >> 8;
    OCR4C = contador_periodo;
    contador_largura_do_pulso = round(contador_periodo/2);
    TC4H = contador_largura_do_pulso >> 8;
    OCR4D = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)-round(largura_do_pulso_desejada*(16000000/256));
    if (contador_largura_do_pulso == round(contador_periodo/2))
    {
      contador_largura_do_pulso = contador_largura_do_pulso - 1;
    }
    TC4H = contador_largura_do_pulso >> 8;
    OCR4B = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)+round(largura_do_pulso_desejada*(16000000/256));
    if (contador_largura_do_pulso == round(contador_periodo/2))
    {
      contador_largura_do_pulso = contador_largura_do_pulso + 1;
    }
    TC4H = contador_largura_do_pulso >> 8;
    OCR4A = contador_largura_do_pulso;
    TCCR4E = 0b00000000;
  }

   // FREQUENCY BETWEEN 122.19 HZ AND 244.38 HZ:
   
  if ((frequencia_desejada >= 122.19)&&(frequencia_desejada <= 244.38))
  {
    TCCR4E = 0b10000000;
    TCCR4A = 0b01010011;
    TCCR4B = 0b00001000; // In this case, timer / counter prescaler 4 is set to 128.
    TCCR4C = 0b01010101;
    contador_periodo = round((16000000/128)/frequencia_desejada);
    TC4H = contador_periodo >> 8;
    OCR4C = contador_periodo;
    contador_largura_do_pulso = round(contador_periodo/2);
    TC4H = contador_largura_do_pulso >> 8;
    OCR4D = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)-round(largura_do_pulso_desejada*(16000000/128));
    if (contador_largura_do_pulso == round(contador_periodo/2))
    {
      contador_largura_do_pulso = contador_largura_do_pulso - 1;
    }
    TC4H = contador_largura_do_pulso >> 8;
    OCR4B = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)+round(largura_do_pulso_desejada*(16000000/128));
    if (contador_largura_do_pulso == round(contador_periodo/2))
    {
      contador_largura_do_pulso = contador_largura_do_pulso + 1;
    }
    TC4H = contador_largura_do_pulso >> 8;
    OCR4A = contador_largura_do_pulso;
    TCCR4E = 0b00000000;
  }

   // FREQUENCY BETWEEN 244.38 HZ AND 300 HZ:
   
    if ((frequencia_desejada >= 244.38)&&(frequencia_desejada <= 300))
  {
    TCCR4E = 0b10000000;
    TCCR4A = 0b01010011;
    TCCR4B = 0b00000111; // In this case, timer / counter prescaler 4 is set to 64.
    TCCR4C = 0b01010101;
    TCCR4D = 0b00000000;
    contador_periodo = round((16000000/64)/frequencia_desejada);
    TC4H = contador_periodo >> 8;
    OCR4C = contador_periodo;
    contador_largura_do_pulso = round(contador_periodo/2);
    TC4H = contador_largura_do_pulso >> 8;
    OCR4D = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)-round(largura_do_pulso_desejada*(16000000/64));
    TC4H = contador_largura_do_pulso >> 8;
    OCR4B = contador_largura_do_pulso;
    contador_largura_do_pulso = round(contador_periodo/2)+round(largura_do_pulso_desejada*(16000000/64));
    TC4H = contador_largura_do_pulso >> 8;
    OCR4A = contador_largura_do_pulso;
    TCCR4E = 0b00000000;
  
  }
} */
