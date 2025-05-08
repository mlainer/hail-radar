FROM debian:bullseye-slim

WORKDIR /app

# Install dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    cmake \
    git\
    libmariadb-dev-compat \
    libmariadb-dev \
    nlohmann-json3-dev \
    wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
COPY hail_log_db.cpp /app/hail_log_db.cpp
#COPY json.hpp /app/json.hpp

# Compile the application
RUN g++ -o hail_log_db hail_log_db.cpp -lmariadb -std=c++11

# Run the application
CMD ["./hail_log_db"]

