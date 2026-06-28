# Relay Run

## Validation
- Command validation
- State persistence
- Error handling

### Error handling
- InvalidCommand
- InvalidEndpoint
- Unreachable
- InvalidValue
- TimeoutExceeded

## Per Device Commands
service/*
relayrun-934833.local/status


## Per Relay Commands
service/relay_number/*
example;

relayrun-934833.local/1/open
relayrun-934833.local/1/closed
relayrun-934833.local/1/toggle
relayrun-934833.local/1/powercycle/1000ms
relayrun-934833.local/1/status


