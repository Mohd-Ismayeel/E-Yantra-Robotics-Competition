import matplotlib.pyplot as plt

# PID parameters (only P used here)
Kp = 20.4  # try different values

# Simulation setup
setpoint = 50
temperature = 20
ambient = 20
history = []

for t in range(200):
    error = setpoint - temperature
    heater_power = Kp * error   # Proportional control
    
    # Simulate effect of heater (proportional heating & natural cooling)
    temperature += heater_power * 0.05          # heater effect
    temperature -= 0.02 * (temperature - ambient)  # cooling effect
    
    history.append(temperature)

# Plot
plt.plot(history, label="Temperature (P-control)")
plt.axhline(setpoint, color='r', linestyle='--', label="Setpoint")
plt.legend()
plt.show()
