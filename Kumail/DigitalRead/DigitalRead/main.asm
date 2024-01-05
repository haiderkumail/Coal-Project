; This example program reads the state of a push button
; if the push button is pressed then turn ON the LED
; if the putton is released then turn OFF the LED

.include "m328pdef.inc"
.include "delay_Macro.inc"
.include "UART_Macros.inc"
.include "div_Macro.inc"

.cseg

.def A  = r16			; just rename or attach a label to the register
.def AH = r17			; just rename or attach a label to the register
.def FlameBuzzer = r18
.def ThermalBuzzer = r19



.org 0x0000

	SBI DDRB, PB5		; PB5 set as OUTPUT Pin
	CBI PORTB, PB5		; LED OFF
	CBI DDRD, PD2		; PB3 set as INPUT pin
	;SBI PORTB, PB3		; Enable internal pull-up resistor

	SBI DDRD, PD7		; PD7 set as OUTPUT Pin
	CBI PORTD, PD7		; buzzer OFF

	SBI DDRD, PD6		; PB6 set as OUTPUT Pin
	CBI PORTD, PD6		; LED OFF

;	CBI DDRD, PD5		; PD5 set as INPUT pin



	; ADC Configuration
	LDI   A,0b11000111		; [ADEN ADSC ADATE ADIF ADIE ADIE ADPS2 ADPS1 ADPS0]
	STS   ADCSRA,A			
	LDI   A,0b01100000		; [REFS1 REFS0 ADLAR – MUX3 MUX2 MUX1 MUX0]
	STS   ADMUX,A			; Select ADC0 (PC0) pin
	;SBI   PORTC,PC0			; Enable Pull-up Resistor

Serial_begin			; initilize UART serial communication

loop:
	delay 1000

	call loadThermalVal
	Serial_writeReg_ASCII AH	; sending the received value to ESP
	cpi AH,138 ; compare LDR reading with our desired threshold
	brlo OnThermalOn ; jump if lessthan (AH < 135)

			; if temperature cold / low --> val == high
				CBI PORTB, PB5	; LED OFF
				CBI PORTD, PB6		; LED OFF
				rjmp flameSection

			; on temp hot
			onThermalOn:
				SBI PORTB, PB5	; LED ON
				SBI PORTD, PB6		; LED ON

				cpi AH,130 ; compare LDR reading with our desired threshold
				brsh FlameSection ; jump if greater than
				LDI ThermalBuzzer, 1


FlameSection:
	; check if flame is On
	SBIC PIND, PD2	; if flame 0, skip next line
	rjmp IfFlameOn
			; if flame not on
				CBI PORTB, PB5	; LED OFF			
				rjmp EndLoop

			; if flame on
			IfFlameOn:
				SBI PORTB, PB5	; LED ON
				LDI FlameBuzzer, 1

EndLoop:

Serial_writeChar '-'


	cpi FlameBuzzer, 1
	breq onBuzzerOn
	cpi ThermalBuzzer, 1
	breq onBuzzerOn
	Serial_writeChar '0'
	rjmp onBuzzerOff

	onBuzzerOn :
		Serial_writeChar '1'
		SBI PORTD, PD7		; buzzer On

	onBuzzerOff :
		

;delay 500
ReadFromEsp:
	LDI r16, 0
	; Check UART serial input buffer for any incoming data and place in r16
	Serial_read
	; If there is no data received in UART serial buffer (r16==0)
	; then don't send it to UART
	CPI r16, 0
	BREQ skip_UART

	CPI r16, 'B'
	breq externalTurnBuzzerOff
;	rjmp skip_UART
;
;
	SBI PORTD, PD7		; buzzer On
	LDI ThermalBuzzer, 1
	LDI FlameBuzzer, 1
	;delay 2000
	
	rjmp skip_UART

externalTurnBuzzerOff:
	CBI PORTD, PD7		; buzzer OFF
	LDI ThermalBuzzer, 0
	LDI FlameBuzzer, 0

	skip_UART:

	rjmp loop


loadThermalVal:

	LDS 	A,ADCSRA		; Start Analog to Digital Conversion
	ORI 	A,(1<<ADSC)
	STS	ADCSRA,A		

	wait:
		LDS 	A,ADCSRA	; wait for ADC conversion to complete
		sbrc 	A,ADSC
	rjmp	wait

	LDS 	A,ADCL			; Must Read ADCL before ADCH
	LDS 	AH,ADCH

ret






; ***************************************************************************
; *	Code written by:														*
; *		Syed Tehseen ul Hasan Shah											*
; *		Lecturer, University of Engineering and Technology Lahore, Pakistan	*
; *		24-December-2023													*
; ***************************************************************************