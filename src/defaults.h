#ifndef SERIAL_BAUD
#define SERIAL_BAUD 115200
#endif
#ifndef SERIAL_DEBUG_ENABLE
#define SERIAL_DEBUG_ENABLE true
#endif

#ifndef CPU_FREQ_MHZ
#define CPU_FREQ_MHZ 80
#endif
#ifndef CPU_FREQ_MHZ_NORADIO
#define CPU_FREQ_MHZ_NORADIO 20
#endif

#define I2C_FREQ_STD 100000
#define I2C_FREQ_FAST 400000
#define I2C_FREQ_SLOW 10000
#define I2C_FREQ_SUPER 1000000

#ifndef I2C_FREQ_DEFAULT
#define I2C_FREQ_DEFAULT I2C_FREQ_STD
#endif