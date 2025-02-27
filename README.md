# Proxy-Web-Server
Advanced Computer Networks Project: Proxy Web Server

/proxy_server
├── src
│   ├── main.c            // Entry point: setup server socket, accept connections
│   ├── server.c          // Network listener and accept loop
│   ├── thread_pool.c     // Thread pool implementation using pthreads
│   ├── http_parser.c     // HTTP request parsing and handling
│   ├── connection_handler.c // Handles individual client connections
│   └── https_tunnel.c    // Handles CONNECT method and SSL tunneling
├── include
│   ├── server.h
│   ├── thread_pool.h
│   ├── http_parser.h
│   ├── connection_handler.h
│   └── https_tunnel.h
├── Makefile              // Build configuration
└── README.md             // Project overview and build instructions

