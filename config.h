
#define F_CPU 8E6

/* PORTB */

#define SPI_PORT PORTB
#define SPI_DDR DDRB
#define SPI_PIN PINB

#define SPI_SCK PB7
#define SPI_MISO PB6						// data received from slave devices
#define SPI_MOSI PB5						// data sent to slave devices


#define BUTTONS_PORT PORTA
#define BUTTONS_DDR DDRA
#define BUTTONS_PIN PINA

#define FM_RESET PD7
#define FM_PORTRESET  PORTD
#define FM_DDRRESET   DDRD
#define FM_SDIO  PC1
#define FM_PORTSDIO  PORTC
#define FM_DDRSDIO   DDRC

#define TW_DDR DDRC
#define TW_PIN PINC
#define TW_PORT PORTC
#define TW_SCK PC0
#define TW_SDA PC1
