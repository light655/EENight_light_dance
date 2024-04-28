#ifndef SETTING_H
#define SETTING_H
const float current_min[4] = {0.03f, 0.03f, 0.01f, 0.01f};
const float control_gain[4] = {50.0f, 60.0f, 40.0f, 20.0f};
const float derivative_gain[4] = {1000.0f, 1000.0f, 6000.0f, 4000.0f};
const float current_max[4] = {0.45f, 0.45f, 0.15f, 0.25f};
#endif