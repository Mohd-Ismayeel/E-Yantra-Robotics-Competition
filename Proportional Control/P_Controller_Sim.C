#include <stdio.h>

int main() {
    double Kp = 21.00001;
    double setpoint = 50.0;
    double temperature = 20.0;
    double ambient = 20.0;

    for (int t = 0; t < 200; t++) {
        double error = setpoint - temperature;
        double heater_power = Kp * error;

        // simulate heating and cooling
        temperature += heater_power * 0.05;
        temperature -= 0.02 * (temperature - ambient);

        printf("Time %d, Temp: %.2f\n", t, temperature);
    }

    return 0;
}
