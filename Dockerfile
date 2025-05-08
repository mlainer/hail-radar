FROM debian:bullseye-slim

WORKDIR /app

# Install dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    cmake \
    git\
    nlohmann-json3-dev \
    wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
COPY . /app
#COPY json.hpp /app/json.hpp

# Compile the application
RUN make

# Run the application
CMD ["./start_hail_radar"]

