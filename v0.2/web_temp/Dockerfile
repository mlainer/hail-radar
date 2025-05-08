FROM debian:bullseye-slim

WORKDIR /app

COPY . /app

RUN apt-get update && \
    apt-get install -y python3 python3-pip && \
    pip install flask mysql-connector-python

CMD ["python3", "app.py"]

