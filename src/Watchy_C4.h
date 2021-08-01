#ifndef WATCHY_C4_H
#define WATCHY_C4_H

#include <Watchy.h>

class WatchyC4 : public Watchy{
    private:
        uint16_t lfsr = 0xAACEu;
    public:
        WatchyC4();
        void centerjustify(const char *str);
        void wordwrap(const char *str, int width, int lineheight, int maxLines);
        void drawWatchFace();
        void drawCall();
        void drawTime();
        void drawDate();
        void drawSteps();
        void drawWeather();
        void drawBattery();
    private:
    void seed_random();
    unsigned short get_random_bit();
    uint16_t get_random_bits(unsigned short n);
    uint16_t get_random_int(uint16_t max);
};

#endif
