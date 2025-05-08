# Use the official Python 3.7.6 image from the Docker Hub
FROM python:3.7.6-slim

# Set the working directory in the container
WORKDIR /app

# Copy the requirements file into the container at /app
COPY requirements.txt .

RUN apt-get update && apt-get install -y sqlite3 netcat

# Install the dependencies specified in requirements.txt
RUN pip install --no-cache-dir -r requirements.txt

# Copy the entire processor directory into the container at /app/processor
COPY processor /app/processor

# Set the working directory to where your main script resides
WORKDIR /app/processor

# Command to run the application
CMD ["python", "ops_hail_radar.py"]
