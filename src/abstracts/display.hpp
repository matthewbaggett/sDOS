#include "kern_inc.h"
#include "driver.hpp"

class AbstractDisplay  : public sDOS_Abstract_Driver
{
    public:
        void writePixel(uint16_t x, uint16_t y, uint16_t value);
};