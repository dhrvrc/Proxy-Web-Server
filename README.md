# Proxy-Web-Server
Advanced Computer Networks Project: Proxy Web Server

/proxy_server  
├── src  
│   ├── main.c                  // Entry point: initializes config, logging, and starts the server.  
│   ├── server.c                // Network listener and accept loop.  
│   ├── thread_pool.c           // Thread pool implementation using p_threads.  
│   ├── http_parser.c           // HTTP request parsing and handling.  
│   ├── connection_handler.c    // Handles individual client connections.  
│   ├── https_tunnel.c          // Handles CONNECT method and HTTPS tunneling.  
│   ├── logging.c               // Logging and error reporting module.  
│   ├── config.c                // Configuration loading and management.  
│   └── utils.c                 // Utility functions (e.g., string helpers, memory wrappers).  
├── include  
│   ├── server.h  
│   ├── thread_pool.h  
│   ├── http_parser.h  
│   ├── connection_handler.h  
│   ├── https_tunnel.h  
│   ├── logging.h  
│   ├── config.h  
│   └── utils.h  
├── tests  
│   ├── test_server.c  
│   ├── test_thread_pool.c  
│   ├── test_http_parser.c  
│   ├── test_connection_handler.c  
│   └── test_https_tunnel.c  
├── docs
│   ├── design.md             // Detailed design and architecture documentation.  
│   └── usage.md              // Usage instructions and examples.  
├── Makefile                  // Build configuration.  
└── README.md                 // Project overview and build instructions.  


