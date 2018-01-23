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
