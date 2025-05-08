-- Create the new database
CREATE DATABASE IF NOT EXISTS hail_speed_db;

-- Use the new database
USE hail_speed_db;

-- Create the hail_speed table
CREATE TABLE IF NOT EXISTS hail_speed (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp TIMESTAMP(6) DEFAULT CURRENT_TIMESTAMP(6),
    frequency DOUBLE NOT NULL,
    speed_kmh DOUBLE NOT NULL
);

