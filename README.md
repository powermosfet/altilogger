# Altilogger

## Altitude logger for RC aircraft

Records altitude using a BME280 sensor, and stores the data in flash to later be transferred to a computer

- `arduino-cli core update-index`
- `arduino-cli compile`
- `arduino-cli upload --port /dev/ttyUSB0`
- `arduino-cli monitor --port /dev/ttyUSB0 --config 115200`
