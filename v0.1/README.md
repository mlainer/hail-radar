Project Name: Hail Radar and MQTT Logger
=================

Description
-----------
This project consists of two main services: `ops_hail_radar` and `mqtt_logger`. The `ops_hail_radar` service configures and initializes the OPS7243 hail radar, while the `mqtt_logger` service logs messages from an MQTT broker (where the radar is sending the data) into a SQLITE3 database. These services are managed using Docker Compose to ensure consistent and isolated environments.

Features
-----------
- `ops_hail_radar`: Configures and initializes the OPS7243 hail radar.
- `mqtt_logger`: Logs messages from an MQTT broker and depends on the `ops_hail_radar` service.

Installation
-----------
To set up the project locally, follow these steps:

1. **Clone the repository**:
   ```bash
   git clone https://github.com/mlainer/hail_radar.git
   cd hail_radar

2. **Ensure Docker and Docker Compose are installed**:
   - [Docker installation guide](https://docs.docker.com/get-docker/)
   - [Docker Compose installation guide](https://docs.docker.com/compose/install/)

3. **Build and start the services using Docker Compose**:
    ```bash
    docker-compose up --build
    ```
    This command will build the Docker images for both services and start the containers.

4. **Stop the services using Docker Compose**:

    To stop the services, use:
    ```bash
    docker-compose down
