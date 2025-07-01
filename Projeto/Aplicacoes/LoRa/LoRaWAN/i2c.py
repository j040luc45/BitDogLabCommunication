from machine import Pin, SoftI2C

# Configure I2C pins (adjust if needed)
i2c = SoftI2C(scl=Pin(3), sda=Pin(2))

# Scan for devices
devices = i2c.scan()

# Print the results
if len(devices) == 0:
    print("No I2C devices found")
else:
    print('I2C devices found:', len(devices))
    for device in devices:
        print("Hexadecimal address:", hex(device))