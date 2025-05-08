# hail-radar 🌩️📡
A modular hail speed tracking and logging system for use in hailstorms utilizing OmniPreSense OPS24x sensors.

Overview
This repository contains a multi-version setup for deploying and testing radar-based hail detection using the OmniPreSense OPS24x radar sensor. It includes serial/USB radar data processing, MQTT/CSV/MariaDB logging, and containerized services for deployment on devices like Raspberry Pi. 

Features
📡 OPS24x Radar Interface
Connects to and reads serial data from the OmniPreSense OPS24x radar module.

🚚 Velocity Tracking
Filters and logs moving objects based on velocity thresholds.

📊 MQTT Logging
Broadcasts radar readings to MQTT for real-time monitoring or IoT integration.

🐳 Dockerized Setup
Includes docker-compose files for easy deployment.

🔧 Network Routing Support
Optional script to route traffic from WiFi to Ethernet on Raspberry Pi.

# Acknowledgments
OmniPreSense for radar hardware
