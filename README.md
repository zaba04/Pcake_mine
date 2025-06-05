# TOR Network Simulation

A simplified implementation of The Onion Router (TOR) network in C, demonstrating anonymous routing through multiple intermediate nodes without encryption.

## Overview

This project simulates the core functionality of TOR networking, where messages are routed through multiple intermediate nodes before reaching their final destination. The system maintains anonymity by ensuring no single node knows the complete path from sender to receiver.

## Architecture

The network consists of three types of components:

- **Clients**: Originate messages and specify routing paths
- **Intermediate Nodes**: Forward messages along the specified route
- **Exit Nodes**: Communicate with final destinations and route responses back

## Features

- ✅ UDP-based message routing
- ✅ Multi-threaded node operation
- ✅ Connection tracking for response routing
- ✅ Support for multiple concurrent clients
- ✅ Configurable routing paths
- ✅ Built-in test servers for demonstration
- ✅ Real-time message tracing and logging

## Project Structure

```
├── main.c              # Network setup and test orchestration
├── tor_types.h         # Core data structures and constants
├── node.h / node.c     # Node implementation (intermediate & exit)
├── client.h / client.c # Client functionality
├── network_utils.h / network_utils.c # Networking utilities
├── test_server.c       # Test server implementation
└── polecenie.txt       # Project requirements (Polish)
```

## Building and Running

### Prerequisites

- GCC compiler
- POSIX-compliant system (Linux/macOS)
- pthread library

### Compilation

```bash
gcc -o tor_network main.c node.c client.c network_utils.c -lpthread
```

```bash
make
```

### Execution

```bash
./tor_network
```

## Network Configuration

### Default Network Topology

- **Test Servers**: Ports 8080 (HTTP-like), 8443 (HTTPS-like)
- **Intermediate Nodes**: Ports 8001 (P1), 8002 (P2)
- **Exit Node**: Port 8003
- **Clients**: Ports 9001, 9002

### Message Flow Example

```
Client 9001 → Node 8001 → Node 8002 → Exit 8003 → Server 8080
            ←           ←           ←         ← Response
```

## Usage Examples

### Basic Message Routing

The system automatically demonstrates several routing scenarios:

1. **HTTP Request Simulation**
   ```
   Route: [8001, 8002, 8003] → 8080
   Data: "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"
   ```

2. **Alternative Route**
   ```
   Route: [8002, 8001, 8003] → 8443
   Data: "Hello HTTPS test server!"
   ```

### Custom Routing

To send custom messages, modify the route array in `main.c`:

```c
int custom_route[] = {8001, 8002, 8003};
send_tor_message(client_port, custom_route, 3, destination_port, "Your message");
```

## Key Data Structures

### Message Structure
```c
typedef struct {
    int msg_type;                    // MSG_DATA or MSG_RESPONSE
    int connection_id;               // Unique connection identifier
    int route_length;                // Number of nodes in route
    int route[MAX_ROUTE_LENGTH];     // Array of node ports
    int final_destination_port;      // Target server port
    char data[BUFFER_SIZE];          // Message payload
    int data_length;                 // Payload size
} Message;
```

### Node Structure
```c
typedef struct {
    int port;                        // Node's listening port
    int node_type;                   // NODE_INTERMEDIATE or NODE_EXIT
    int socket_fd;                   // UDP socket descriptor
    ConnectionInfo connections[MAX_CONNECTIONS]; // Active connections
    pthread_t thread;                // Node thread handle
} Node;
```

## How It Works

### Forward Path (Client → Destination)

1. **Client** creates message with complete route specification
2. **First Node** receives message, stores sender info, forwards to next node
3. **Intermediate Nodes** repeat the forwarding process
4. **Exit Node** communicates with actual destination server

### Return Path (Destination → Client)

1. **Exit Node** receives response from destination
2. **Each Node** uses stored connection info to route response backward
3. **Client** receives final response

### Connection Tracking

Each node maintains a connection table:
```c
typedef struct {
    int connection_id;    // Unique identifier
    int prev_node_port;   // Where message came from
    int is_active;        // Connection status
} ConnectionInfo;
```

## Configuration Constants

```c
#define MAX_NODES 10            // Maximum network nodes
#define MAX_CONNECTIONS 100     // Connections per node
#define BUFFER_SIZE 1024        // Message buffer size
#define MAX_ROUTE_LENGTH 5      // Maximum routing hops
```

## Testing

The system includes built-in test servers that respond to incoming messages:

- **Port 8080**: HTTP-like responses
- **Port 8443**: Generic acknowledgment responses

### Sample Output

```
Test server listening on port 8080
Node 8001 (Intermediate) listening on port 8001
Node 8002 (Intermediate) listening on port 8002
Node 8003 (Exit) listening on port 8003

Client 9001: Sending message through TOR network
Route: 8001 8002 8003 -> 8080
Intermediate node 8001: Received message from port 9001
Intermediate node 8001: Forwarding to port 8002
...
Exit node 8003: Sending to final destination port 8080
Test server 8080: Received: GET / HTTP/1.1...
Client 9001: Received response: HTTP/1.1 200 OK...
```

## Limitations

- **No Encryption**: Messages are transmitted in plaintext
- **Local Network Only**: All communication via localhost
- **UDP Only**: No TCP support
- **Basic Error Handling**: Minimal fault tolerance
- **Fixed Topology**: Network structure is predefined

## Security Considerations

This is a **simulation for educational purposes only**. It lacks:
- Message encryption
- Node authentication
- Traffic analysis protection
- Secure key exchange
- Production-grade error handling

## Future Enhancements

- [ ] Message encryption/decryption
- [ ] Dynamic node discovery
- [ ] TCP support
- [ ] Enhanced error handling and recovery
- [ ] Network topology visualization
- [ ] Performance metrics and monitoring
- [ ] Configuration file support

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This project is created for educational purposes. Please refer to your institution's policies regarding code usage and distribution.

## References

- [TOR Project Official Documentation](https://www.torproject.org/)
- [Onion Routing Protocol Specification](https://spec.torproject.org/)
- Original requirements document: `polecenie.txt`

