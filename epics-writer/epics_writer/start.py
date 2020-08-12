import json
from pika import BlockingConnection, ConnectionParameters, BasicProperties

from epics_writer.writer import write_epics_pvs


DEFAULT_BROKER_URL = "127.0.0.1"
REQUEST_EXCHANGE = "request"
STATUS_EXCHANGE = "status"
QUEUE_NAME = "epics"
OUTPUT_FILE_SUFFIX = ".PVCHANNELS.h5"


def update_status(channel, body, action, file, message=None):
    status_header = {
        "action": action,
        "source": "epics_writer",
        "routing_key": QUEUE_NAME,
        "file": file,
        "message": message
    }

    channel.basic_publish(exchange=STATUS_EXCHANGE,
                          properties=BasicProperties(
                              headers=status_header),
                          routing_key="",
                          body=body)


def on_message(channel, method_frame, header_frame, body):

    output_file = None

    try:
        request = json.loads(body.decode())
        output_prefix = request["output_prefix"]
        start_pulse_id = 1
        stop_pulse_id = 10
        metadata = request["metadata"]
        epics_pvs = request["epics_pvs"]

        output_file = output_prefix + OUTPUT_FILE_SUFFIX

        update_status(channel, body, "write_start", output_file)

        write_epics_pvs(output_file=output_file,
                        start_pulse_id=start_pulse_id,
                        stop_pulse_id=stop_pulse_id,
                        metadata=metadata,
                        epics_pvs=epics_pvs)

    except Exception as e:
        channel.basic_reject(delivery_tag=method_frame.delivery_tag,
                             requeue=False)

        update_status(channel, body, "write_rejected", output_file, str(e))

    else:
        channel.basic_ack(delivery_tag=method_frame.delivery_tag)

        update_status(channel, body, "write_finished", output_file)


def connect_to_broker(broker_url):
    connection = BlockingConnection(ConnectionParameters(broker_url))
    channel = connection.channel()

    channel.exchange_declare(exchange=STATUS_EXCHANGE,
                             exchange_type="fanout")
    channel.exchange_declare(exchange=REQUEST_EXCHANGE,
                             exchange_type="topic")

    channel.queue_declare(queue=QUEUE_NAME, auto_delete=True)
    channel.queue_bind(queue=QUEUE_NAME,
                       exchange=REQUEST_EXCHANGE,
                       routing_key="*.%s.*" % QUEUE_NAME)
    channel.basic_qos(prefetch_count=1)
    channel.basic_consume(QUEUE_NAME, on_message)

    try:
        channel.start_consuming()
    except KeyboardInterrupt:
        channel.stop_consuming()


def main():
    import argparse
    parser = argparse.ArgumentParser(description='Epics HDF5 writer')
    parser.add_argument('--broker_url', dest='broker_url',
                        default=DEFAULT_BROKER_URL,
                        help='RabbitMQ broker URL')

    args = parser.parse_args()

    connect_to_broker(broker_url=args.broker_url)


if __name__ == '__main__':
    main()
