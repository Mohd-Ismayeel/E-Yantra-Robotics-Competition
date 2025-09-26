import matplotlib.pyplot as plt

# Simulation setup
setpoint = 50
temperature = 20
ambient = 20
heater_on = False

history = []

for t in range(100):
    # Bang-bang control
    if temperature < setpoint:
        heater_on = True
    else:
        heater_on = False
    
    # Effect of heater
    if heater_on:
        temperature += 5  # heat rate
    else:
        temperature -= 2  # cooling rate
    
    # Store data
    history.append(temperature)

# Plot
plt.plot(history, label="Temperature")
plt.axhline(setpoint, color='r', linestyle='--', label="Setpoint")
plt.legend()
plt.show()
