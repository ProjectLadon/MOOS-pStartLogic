# MOOS-pStartLogic
MOOS start/stop logic for enabling & disabling propulsion

# Requirements & Dependencies
* rapidjson (https://github.com/Tencent/rapidjson/)

# Theory of Operation
This module provides an start/emergency stop signal interface that activates and deactivates propulsion. It also optionally provides a warning signal to drive a horn or similar warning device.

This module takes a start signal, a stop signal, and one or more signals to be re-transmitted. In the starting condition, the re-transmitted signals are replaced by default safe values specified at configuration time.

When the start condition goes to the given true value for a configured time period, the system sets the warning signal true for a configurable time period and then starts re-transmitting the re-transmitted signals in active mode.

If, during the warning or active periods, the stop signals goes true, this module goes back into start mode.

# Configuration

## START_CONDITION
This is the definition of a start condition. Multiple start conditions are allowed. Each is defined by a json string conforming to the following schema:
```
{
    "$schema": "http://json-schema.org/schema#",
	"id": "StartConditionJSON",
	"type": "object",
    "properties": {
        "start_message": {"type": "string"},
        "start_delay": {"type": "integer"},
        "truth_conditions": {
            "type": "array",
            "items": {
                "anyOf": [
                    {"type": "string"},
                    {"type": "integer"}
                ]}
            }
        }
    }
}
```

### start_message
This is the name of the message that this module uses to determine whether to start or not.

### start_delay
This is the time period, in milliseconds, that this condition must remain true to trigger a state transition. This is measured between message arrivals, so the message must arrive at least twice.

### truth_condition
This is a list of numbers and/or strings that this module will interpret as truth conditions for this message.

## STOP_CONDITION
This is the definition of a stop condition. Multiple stop conditions are allowed. Each is defined by a json string conforming to the following schema:
```
{
    "$schema": "http://json-schema.org/schema#",
	"id": "StopConditionJSON",
	"type": "object",
    "properties": {
        "stop_message": {"type": "string"},
        "truth_conditions": {
            "type": "array",
            "items": {
                "anyOf": [
                    {"type": "string"},
                    {"type": "integer"}
                ]}
            }
        }
    }
}
```

### stop_message
This is the name of the message that this module uses to determine whether to stop or not.

### truth_condition
This is a list of numbers and/or strings that this module will interpret as truth conditions for this message.

## WARNING_MESSAGE
This is the output that the module transmits when it's in the warning state. Multiple warning messages are allowed. Each is defined by a JSON string corresponding to the following schema:
```
{
    "$schema": "http://json-schema.org/schema#",
	"id": "WarningMessageJSON",
	"type": "object",
    "properties": {
        "warning_msg": {"type": "string"},
        "on_output": {
            "anyOf": [
                {"type": "string"},
                {"type": "integer"}
            ]
        },
        "off_output": {
            "anyOf": [
                {"type": "string"},
                {"type": "integer"}
            ]
        }
    }
}                
```

## RUNNING_MESSAGE
This is the output that the module transmits when it's in the running state. Multiple running messages are allowed. Each is defined by a JSON string corresponding to the following schema:
```
{
    "$schema": "http://json-schema.org/schema#",
	"id": "RunningMessageJSON",
	"type": "object",
    "properties": {
        "running_msg": {"type": "string"},
        "on_output": {
            "anyOf": [
                {"type": "string"},
                {"type": "integer"}
            ]
        },
        "off_output": {
            "anyOf": [
                {"type": "string"},
                {"type": "integer"}
            ]
        }
    }
}                
```

## RETRANSMIT_MESSAGE
This is the output that the module transmits when it's in the running state. Multiple running messages are allowed. Each is defined by a JSON string corresponding to the following schema:
```
{
    "$schema": "http://json-schema.org/schema#",
	"id": "RetransmitMessageJSON",
	"type": "object",
    "properties": {
        "input_msg": {"type": "string"},
        "output_msg": {"type": "string"},
        "default_msg": {
            "anyOf": [
                {"type": "string"},
                {"type": "number"}
            ]
        }
    }
}                
```
