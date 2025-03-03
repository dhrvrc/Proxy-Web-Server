# Proxy-Web-Server

This is a multi-threadedHTTP/HTTPS proxy. It features request handling, URL blocking, response caching, and a management console for administrative actions. The core is written in C, with a simple Flask (Python) Web Application for the admin/management console.


## Project Organisation

/proxy_server  
├── src  
│   ├── main.c                  // Entry point: initializes config, logging, and starts the server.    
│   ├── proxy.c                 
│   ├── thread_pool.c           
│   ├── http_handler.c         
│   ├── cache.c                 
│   ├── console.c                 
│   ├── management_console.c                 
│   └── logging.c               
├── include  
│   ├── proxy.h  
│   ├── thread_pool.h  
│   ├── http_handler.h  
│   ├── cache.h  
│   ├── console.h  
│   ├── management_console.h  
│   └── logging.h  
├── tests  
│   ├──  
├── docs  
│   ├──   
├── Makefile                  // Build configuration.  
└── README.md                  


