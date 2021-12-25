# arduino-esp8266-wifi
scripts for communicating esp8266 controllers over wifi

## trouble shooting
- `FileNotFoundError: [Errno 2] No such file or directory: '/dev/ttyUSB0'`
  - check your pot in arduino studio Tools => Port
    - there you see something like ttyS0
  - `sudo chmod a+rw /dev/ttyS0`