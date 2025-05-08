import mqtt_logger
import os
import time

# Initalise mqtt recorder object
rec = mqtt_logger.Recorder(
    sqlite_database_path=os.path.join(os.path.dirname(__file__), "MQTT_log.db"),
    topics=["A23170066/#"],
    broker_address="omnipresense.net",
    verbose=True,

    ## Uncomment for TLS connection
    # port=8883,
	# use_tls=True,
	# tls_insecure=False,

    ## Uncomment for username and password
    username="4P944_08",
    password="76&tu*!G",
)

# Start the logger
rec.start()
time.sleep(60*60*1)
rec.stop()
