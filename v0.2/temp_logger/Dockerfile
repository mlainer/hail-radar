# Use an official Python runtime as a parent image
FROM debian:bullseye-slim

# Set the working directory
WORKDIR /app

# Copy the current directory contents into the container at /app
COPY . /app

# Install the MySQL client and Python dependencies
RUN apt-get update && \
    apt-get install -y python3 python3-pip && \
    pip install mysql-connector-python

# Run the wait-for-mysql script and then the application
CMD ["python3", "./temp_logger.py"]

