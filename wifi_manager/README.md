# ESP32 WiFi Manager

This directory contains a small standalone WiFi manager for ESP32 projects using ESP-IDF.
Drop `wifi_manager.c` and `wifi_manager.h` into your project and call `wifi_manager_init()` with your network credentials followed by `wifi_manager_start()`.
The manager handles connection and simple reconnection logic.
