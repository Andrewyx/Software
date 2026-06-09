#include "dribbler.h"

Dribbler::Dribbler()
{
    pinMode(DRIBBLER_PIN, OUTPUT);
    analogWrite(DRIBBLER_PIN, 0);
}

void Dribbler::dribble(uint32_t speed)
{
    analogWrite(DRIBBLER_PIN, speed);
}
