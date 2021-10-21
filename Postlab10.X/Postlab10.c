/*
 * Archivo:         Prelab8.c
 * Dispositivo:     PIC16F887
 * Autor:           Francisco Javier López
 *
 * Programa:        Programa que pregunta si desea alterar el PuertoA o el 
 *                  PuertoB, recibe el input, y después permite que se ingrese
 *                  el valor decimal que se asginará al puerto
 * Hardware:        Terminal virtual en TX y RX, Leds en PuertoA y PuertoB
 * 
 * Creado: 19 de octubre de 2021
 * Última Modificación: 20 de octubre de 2021
 */


// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>
#include <string.h>

#define _XTAL_FREQ 1000000

//============================================================================
//============================ VARIABLES GLOBALES ============================
//============================================================================
char pInicial[] = "Que puerto desea cambiar? 1->A, 2->B ";
char pFinal[] = "Escriba el valor a ingresar en el puerto ";
char errorPuerto[] = "El puerto seleccionado no es valido";
uint8_t valor[3];       // Almacenador de valores ingresados
uint8_t valor_dec[3];   // Almacenador de valores traducidos a decimal
uint8_t caso = 0;       // Variable de switch principal
uint8_t puerto = 0;     // 1PortA, 2PortB
uint8_t pos_valor = 0;  // Switch que va de 0 a 2



//============================================================================
//========================= DECLARACIÓN DE FUNCIONES =========================
//============================================================================
void config_io(void);
void config_reloj(void);
void baud_config(void);
void config_int(void);
void enviar_string(char dato[]);
uint8_t traducir (uint8_t num);
uint8_t sumar_vector(uint8_t vect[3]);

//============================================================================
//============================== INTERRUPCIONES ==============================
//============================================================================
void __interrupt() isr (void)
{
    PORTE = 7;
    if(PIR1bits.RCIF)         // Si la bandera está encendida, entrar
    {
        switch(caso)
        {
            case 0:
                // Mostrar output, rutina de main
                break;
            case 1:
                PORTE = 2;
                puerto = RCREG; // puerto recibe el input
                caso = 2;       // Actualizar el switch para el main
                
                if (PIR1bits.TXIF)
                {
                    TXREG = 13; // Enviar salto de línea  
                }
                break;
            case 2:
                // Mostrar output, rutina de main
                break;
            case 3:
                switch(puerto)
                {
                    case 49:        // ASCII 49 = Decimal 1 = Puerto A
                        valor[pos_valor] = RCREG;   // Avanzar 1 a 1 en el vector
                        pos_valor = pos_valor + 1;  // Incrementar cada interrupción
                        __delay_ms(100);
                        
                        if(pos_valor > 2)           // Si ya se ingresaron tres números
                        {
                            pos_valor = 0;          // Reiniciar cuenta
                            
                            valor_dec[2] = traducir(valor[2]);      // Traducir de Ascii a decimal
                            valor_dec[1] = 10*traducir(valor[1]);   // y multiplicar según sea necesario
                            valor_dec[0] = 100*traducir(valor[0]);
                            
                            PORTA = sumar_vector(valor_dec);        // Sumar las tres posiciones
                            
                            caso = 0;
                            
                            if (PIR1bits.TXIF)
                            {
                                TXREG = 13; // Enviar salto de línea  
                            }
                        }
                        break;
                    case 50:        // ASCII 50 = Decimal 2 = Puerto B
                        valor[pos_valor] = RCREG;   // Avanzar 1 a 1 en el vector
                        pos_valor = pos_valor + 1;  // Incrementar cada interrupción
                        __delay_ms(100);
                        
                        if(pos_valor > 2)           // Si ya se ingresaron tres números
                        {
                            pos_valor = 0;          // Reiniciar cuenta
                            
                            valor_dec[2] = traducir(valor[2]);      // Traducir de Ascii a decimal
                            valor_dec[1] = 10*traducir(valor[1]);   // y multiplicar según sea necesario
                            valor_dec[0] = 100*traducir(valor[0]);
                            
                            PORTB = sumar_vector(valor_dec);        // Sumar las tres posiciones
                            
                            caso = 0;
                            
                            if (PIR1bits.TXIF)
                            {
                                TXREG = 13; // Enviar salto de línea  
                            }
                        }
                        break;
                    default:
                        caso = 0;
                        
                        if (PIR1bits.TXIF)
                            {
                                enviar_string(errorPuerto);  
                            }
                        break;
                }
                break;
            default:
                caso = 0;
                break;
        }
    }
}

//============================================================================
//=================================== MAIN ===================================
//============================================================================
void main(void) 
{
    config_io();
    config_reloj();
    baud_config();
    config_int();
    
    while(1)
    {
        __delay_ms(100);
        
        switch(caso)
        {
            case 0:                             // Pregunta 1
                if (PIR1bits.TXIF)              // Reviar bandera de envío
                {
                    enviar_string(pInicial);    // Enviar primera pregunta  
                    caso = 1;                   // Cambiar de caso
                }
                break;
            case 1:
                // Pedir input, rutina de interrupción
                break;
            case 2:                             // Pregunta 2
                if (PIR1bits.TXIF)              // Revisar bandera de envío
                {
                    enviar_string(pFinal);      // Enviar segunda pregunta
                    caso = 3;                   // Cambiar de caso
                }
                break;
            case 3:
                // Pedir input, rutina de interrupción
                break;
            default:
                caso = 0;
                break;
        }
    }
}

//============================================================================
//================================ FUNCIONES =================================
//============================================================================
void config_io(void)            // Configuración de entradas y salidas
{
    ANSEL   =   0;              // Puertos digitales
    ANSELH  =   0;              // Puertos digitales
    TRISA   =   0;              // PuertoA como salida
    TRISB   =   0;              // PuertoB como salida
    PORTA   =   0;              // Limpiar PuertoB
    PORTB   =   0;              // Limpiar PuertoB
    return;
}

void config_reloj(void)         // Configuración del oscilador
{
    OSCCONbits.IRCF2 = 1;       // 1MHz
    OSCCONbits.IRCF1 = 0;       // 
    OSCCONbits.IRCF0 = 0;       // 
    OSCCONbits.SCS = 1;         // Reloj interno
    return;
}

void baud_config(void)
{
    TXSTAbits.SYNC = 0;         // Transmición asíncrona
    TXSTAbits.BRGH = 1;         // Alta velocidad de baudios
    BAUDCTLbits.BRG16 = 1;      // Generador de ratio de baudios de 16 bits
    SPBRGH = 0;                 // Generador a 25 decimal (9615 bauds)
    SPBRG = 25;  
    
    RCSTAbits.SPEN = 1;         // Habilitar puertos seriales
    RCSTAbits.RX9 = 0;          // Recepción de 8 bits
    RCSTAbits.CREN = 1;         // Habilitar receptor
    
    TXSTAbits.TX9 = 0;          // Transmisión de 8 bits
    TXSTAbits.TXEN = 1;         // Habiliar transmisor
    return;
}

void config_int(void)           // Configuración de interrupciones
{
    INTCONbits.GIE  = 1;        // Activar interrupciones
    INTCONbits.PEIE = 1;        // Activar interrupciones periféricas
    PIE1bits.RCIE = 1;          // Habilitar interrupción de recepción
    PIR1bits.RCIF = 0;          // Bajar bandera de recepción
    return;
}


void enviar_string(char dato[])
{
    for(int i = 0; i<strlen(dato); i++) // Ir de 0 hasta la longitud del string
    {
        TXREG = dato[i];    // Mostrar cada char del string de 1 en 1
        __delay_ms(20);     // Delay entre cada char mostrado
    }
    return;
}

uint8_t traducir(uint8_t num)
{
    switch(num)             // Traductor de número de ASCII a decimal
    {
        case 48:
            return 0; 
            break;
        case 49:
            return 1;
            break;
        case 50:
            return 2;
            break;
        case 51:
            return 3;
            break;
        case 52:
            return 4;
            break;
        case 53:
            return 5;
            break;
        case 54:
            return 6;
            break;
        case 55:
            return 7;
            break;
        case 56:
            return 8;
            break;
        case 57:
            return 9;
            break;
        default:
            return 0;
            break;
    }
}

uint8_t sumar_vector(uint8_t vect[3])   // Sumar los tres espacios del array
{
    uint8_t resultado;
    resultado = vect[0] + vect[1] + vect[2];    
    return resultado;
}