docker run -itd —net host -p 0.0.0.0:51826:51826 $(find /dev/snd/ -type c | sed ‘s/^/ — device /’) -v /path/to/homebridge/config/folder/:/root/.homebridge hugoch/rpi-python-homebridge:v1 /root/run_homebridge.sh

