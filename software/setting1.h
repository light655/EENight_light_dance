#ifndef SETTING_H
#define SETTING_H
const float current_min[4] = {0.03f, 0.03f, 0.01f, 0.03f};
const float control_gain[4] = {50.0f, 50.0f, 50.0f, 50.0f};
const float derivative_gain[4] = {1000.0f, 1000.0f, 1000.0f, 1000.0f};
const float current_max[4] = {0.45f, 0.45f, 0.15f, 0.35f};
const float voltage_current_factor[4] = {0.0f, 0.0f, 16.0f, 9.0f};
#endif