# Stereo Vision System (Raspberry Pi 5)
**Dual VL53L8CX ToF Sensors + Dual RGB Cameras UDP/TCP Streamer**

This repository contains the setup files and C source code to turn a Raspberry Pi 5 into a fully autonomous stereoscopic vision node. It reads depth data from **two VL53L8CX Time-of-Flight sensors** and captures video from **two RGB cameras**, streaming all data concurrently over the local network via UDP and TCP.

## Hardware Requirements
* **Raspberry Pi 5** (Running Raspberry Pi OS 64-bit Bookworm)
* **2x VL53L8CX** (or VL53L5CX) Time-of-Flight Sensors
* **2x Raspberry Pi RGB Cameras** (connected to CAM/DISP 0 and CAM/DISP 1 ports)

## Wiring & Pinout
Since both ToF sensors have the same hardware I2C address (`0x29`), we must connect them to two separate I2C buses on the Raspberry Pi.

* **Left Eye (I2C-1)**
* **Right Eye (I2C-0)**
* **Power & ground for both**

## Source of the Sensor Libraries
All necessary sensor libraries are directly included in this repository so you can compile the project out-of-the-box. 
* The core Ultra Lite Driver (ULD) API files for the VL53L8CX are sourced from the [stm32duino/VL53L8CX](https://github.com/stm32duino/VL53L8CX) repository.
* The `vl53l8cx_platform.c` and `vl53l8cx_platform.h` files have been significantly modified specifically for this project to drop Arduino dependencies and directly utilize the standard Linux `/dev/i2c-x` bus.

## Step 1: Raspberry Pi Configuration
Before compiling the code, you need to enable both hardware I2C buses. You can configure everything directly via the command line:

1. Enable the primary bus (`i2c-1`) using the non-interactive mode:
   ```bash
   sudo raspi-config nonint do_i2c 0
   ```
2. Enable the secondary bus (`i2c-0`) by appending the required parameter to the boot configuration:
   ```bash
   echo "dtparam=i2c_vc=on" | sudo tee -a /boot/firmware/config.txt
   ```
3. **Reboot** your Raspberry Pi to apply the hardware changes:
   ```bash
   sudo reboot
   ```

## Step 2: Installation & Compilation
1. Install the essential compilation tools and I2C utilities:
   ```bash
   sudo apt update
   sudo apt install build-essential i2c-tools git -y
   ```
2. Clone this repository to your home directory (`/home/user/`):
    ```bash
    git clone https://github.com/Project-Humanoid-Head/RPI5-Backend.git
    cd RPI5-Backend/ToF_sensor
    ```
3. Compile the ToF sensor code into an executable binary:
    ```bash
    gcc *.c -o tof_streamer -lm
    ```

## Step 3: Setting Up Automatic Startup (systemd)
To make the Raspberry Pi start the vision system automatically upon booting, we use the provided script and `systemd` service.

1. Navigate to the startup scripts folder:
    ```bash
    cd ../Auto_startup
    ```
2. Make the bash script executable:
    ```bash
    chmod +x start_vision.sh
    ```
3. Copy the service file to the Linux system directory:
    ```bash
    sudo cp robot_vision.service /etc/systemd/system/
    ```
4. Reload the system daemon to recognize the new service:
   ```bash
   sudo systemctl daemon-reload
   ```
5. Enable the service to start automatically on boot:
   ```bash
   sudo systemctl enable robot_vision.service
   ```
6. Start the service right now (to test it without rebooting):
    ```bash
    sudo systemctl start robot_vision.service
    ```

> **⚠️ Important Path Note:** The `start_vision.sh` and `robot_vision.service` files assume your repository is cloned into `/home/user/RPI5-Backend`. If you use a different username or repository name, you must open these two files and update the paths before copying the service to `/etc/systemd/system/`.

## Network Output Details
Once running, the Raspberry Pi will broadcast data constantly on the following ports:

| Hardware | Network Protocol | IP / Address | Port |
| :--- | :--- | :--- | :--- |
| **Left ToF Sensor** | UDP Broadcast | `255.255.255.255` | `5005` |
| **Right ToF Sensor** | UDP Broadcast | `255.255.255.255` | `5006` |
| **RGB Camera 0** | TCP Stream (MJPEG) | `<Raspberry_IP>` | `8888` |
| **RGB Camera 1** | TCP Stream (MJPEG) | `<Raspberry_IP>` | `8889` |

## Troubleshooting
To check if the background service is running correctly:
```bash
sudo systemctl status robot_vision.service
```
To view the live console output (logs) from the cameras and sensors:
```bash
journalctl -u robot_vision.service -f
```
