![Schema Data processing](https://github.com/istik/iot_bigdata/blob/master/pic/projetiot.png)

## Objective

This project covers the process of deploying a simple architecture for the real-time and batch processing a temperature sensor data on Arduino with open source technologies part of the Big Data ecosystem. The purpose of the solution is to exemplify the flow of data through the different tools, from its capture to its transformation and Insights generation.

Within the presented architecture the services of publication, transfer and storage of data are agnostic to the format in which the data is sent by the Arduino board. This drives the idea of building a centralized service for the distribution of messages from different sending devices to many clients or services capable of consuming this data.

## Versions and tools
- Arduino IDE 1.8.5
- Amazon Simple Notification Service
- Amazon CloudWatch
- Amazon S3
- AWS IoT
- Kinesis Firehose
- Kinesis analytics
- MQTT


## Data flow
![Schema data flow](https://github.com/istik/iot_bigdata/blob/master/pic/schema.png)

### Data generation from temperature sensor

The code loaded in the Arduino platform makes readings trough the DS18B20 sensor, capturing the temperature in Celsius (°C).


### Data publication to the AWS IoT via protocol MQTT

The message or payload that will be sent to the AWS Iot via MQTT is built:
The payload is in JSON format.
It contains name of the sensor, data captured by the sensor, date/time of the reading, for exemple:

`{
"device": "temperature1",
"temperature": 24.3125,
"timestamp_t": "1516715705"
}`

The payload is published to the MQTT broker on a specific topic under a predefined username and password.
The MQTT broker has a list of permissions that defines which users can publish information over existing topics.

### Real-time data capture

The AWS IoT service has an organized set of instructions(AWS Iot Rules) that orchestrate the flow of data as they are captured:
AWS IoT connects or subscribes to the MQTT topic and captures messages in real time.
AWS IoT publishes the original message in AWS Kinesis Service.
AWS Kinesis allows real-time processing of data sent by the Arduino platform.

### Data processing
![Schema Data processing](https://github.com/istik/iot_bigdata/blob/master/pic/Capture%20d%E2%80%99e%CC%81cran%202018-01-23%20a%CC%80%2023.04.44.png)

Kinesis runs code blocks (SQL):

`
CREATE OR REPLACE STREAM "DESTINATION_SQL_STREAM" (device VARCHAR(20), temperature FLOAT, timestamp_t TIMESTAMP);
CREATE OR REPLACE PUMP "STREAM_PUMP" AS INSERT INTO "DESTINATION_SQL_STREAM"
 
SELECT "device", "temperature", TO_TIMESTAMP(("timestamp_t"+3600)*1000) as "timestamp_t"
FROM "SOURCE_SQL_STREAM_001"

`

It is possible to subscribe in real time to the AWS IoT MQTT topic to process the messages under different time windows.
The code is executed on SQL.
The data obtained in each time window are transformed and stored in AWS S3.

## Alarm system

![Schema aws sns](https://github.com/istik/iot_bigdata/blob/master/pic/awssns.jpg)

Amazon Simple Notification Service (SNS) is a flexible, fully managed pub/sub messaging and mobile notifications service for coordinating the delivery of messages to subscribing endpoints and clients. 


## Libraries
OneWire 2.3.3

AWS_IOT 1.0.0

NTPClient 3.1.0
