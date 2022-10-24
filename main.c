include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t effect = 0;
uint8_t effect_data = 0;
uint8_t isRun = 1;
uint8_t interface_option = 0;
uint8_t control_option = -1;

const char light_to_centre[] = {0x81, 0x42, 0x24,
  0x18, 0x24, 0x42};

const uint8_t shadow[] = {0xFE, 0xFD, 0xFB, 0xF7,
  0xEF, 0xDF, 0xBF, 0x7F};

const uint8_t jhonson_counter[] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 
  0x7F, 0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80, 0x00};

void USART_Init(uint32_t baud)
{

 UBRR0H = (uint8_t)((F_CPU/(16*baud)-1)>>8);
 UBRR0L = (uint8_t)(F_CPU/(16*baud) - 1);
 UCSR0A = 0;

 UCSR0B = (1 << RXEN0)|(1<<TXEN0);
 

 UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
}

void SPI_Init ()
{
  DDRB |= ((1<<PORTB2)|(1<<PORTB3)|(1<<PORTB5)); 
  PORTB &= ~((1<<PORTB2)|(1<<PORTB3)|(1<<PORTB5));

  SPCR = ((1<<SPE)|(1<<MSTR));
}

void SPI_sendData (uint8_t data)
{
  PORTB &= ~(1<<PORTB2); 

  SPDR = data;
  while(!(SPSR & (1<<SPIF)));

  PORTB |= (1<<PORTB2);
}

void USART_PutChar(uint8_t data)
{

 while (!(UCSR0A & (1<<UDRE0))){ ; }

 UDR0 = data;
}

void print_string(const char *str)
{
 while (*str) {
 USART_PutChar(*str++);
 }
}

void run_effect()
{
  if (isRun == 1)
  {
      switch (effect)
      {
      case 0:
        SPI_sendData(light_to_centre[effect_data]);
        if (++effect_data > 5) effect_data = 0;
        break;

      case 1:
        SPI_sendData(shadow[effect_data]);
        if(++effect_data > 7) effect_data = 0;
        break;

      case 2:
        SPI_sendData(jhonson_counter[effect_data]);
        if(++effect_data > 15) 
        effect_data = 0;
        break;
      }
  }
}

//======================================
int main(void)
{
  uint8_t dl = 0;
  uint8_t effect_delay = 15;
  uint8_t buffer_len = 0;
  char text_buffer[32];
  USART_Init(9600);
  SPI_Init();
  print_string("\nPlease, choose one of the options below:\n 1. Turn ON|OFF\n 2. Change Effect\n 3. Change Speed\n");
  
  while(1)
  {
    if (UCSR0A & (1 << RXC0))
    {
      char b = UDR0;

      if (b == '\n' || b == '\r')
      {
        if (buffer_len > 0)
        {
          switch (interface_option)
          {
          case 0:
            if (0 == strcmp(text_buffer, "1"))
            {
               isRun = !isRun;
               SPI_sendData(0x00);
            } else if (0 == strcmp(text_buffer, "2"))
            {
              interface_option = 1;
              print_string("\n1. Light to centre\n2. Shadow\n3. Jhonson counter\n0. Back to main menu");
            } else if (0 == strcmp(text_buffer, "3"))
            {
              interface_option = 2;
              print_string("\n1. Slow speed\n2. Medium speed\n3. Fast speed\n0. Back to main menu");
            } else
            {
              print_string("\nNo such command\n");
            }
            break;
        
          case 1:
            if (0 == strcmp(text_buffer, "1"))
            {
             effect_data = 0;
              effect = 0;
            } else if (0 == strcmp(text_buffer, "2"))
            {
              effect_data = 0;
              effect = 1;
            } else if (0 == strcmp(text_buffer, "3"))
            {
              effect_data = 0;
              effect = 2;
            } else if (0 == strcmp(text_buffer, "0"))
            {
              interface_option = 0;
              print_string("\nPlease, choose one of the options below:\n 1. Turn ON|OFF\n 2. Change Effect\n 3. Change Speed\n");
            } else
            {
              print_string("\nNo such command\n");
            }
            break;

          case 2:
            if (0 == strcmp(text_buffer, "1"))
            {
             effect_delay = 15;
            } else if (0 == strcmp(text_buffer, "2"))
            {
               effect_delay = 10;
            } else if (0 == strcmp(text_buffer, "3"))
            {
              effect_delay = 5;
            } else if (0 == strcmp(text_buffer, "0"))
            {
              interface_option = 0;
              print_string("\nPlease, choose one of the options below:\n 1. Turn ON|OFF\n 2. Change Effect\n 3. Change Speed\n");
            } else
            {
              print_string("\nNo such command\n");
            }
            break;

        default:
          print_string("No such command, to see instructions type and enter '4'");
          break;
        }

          buffer_len = 0;
        }
      
      } else 
      {
        if (buffer_len < 31)
        {
          text_buffer[buffer_len++] = b;
          text_buffer[buffer_len] = 0;
        }
      }
    }

    if (++dl > effect_delay)
    {
      dl = 0;
      run_effect();
    }

    _delay_ms(10);
  }

  return 0;
}
