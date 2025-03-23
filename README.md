# Proxy-Web-Server

This is a multi-threadedHTTP/HTTPS proxy. It features request handling, URL blocking, response caching (HTTP Only), and a management console for administrative actions. The core is written in C, with a simple Flask (Python) Web Application for the admin/management console.

## Dependencies/Prerequisites
| Dependency | Version     | Install                                   |
|------------|-------------|-------------------------------------------|
| C          | 23 Standard | [docs](https://installc.org/)             |
| GCC*       | 13.2.0      | [docs](https://gcc.gnu.org/install/)**    |
| Python     | 3.11.*      | [docs](https://www.python.org/downloads/) |
| pip        | 24.2        | Should install with Python                |

*Or any C Compiler of your choice.  
**Should generally install with one click C installer.

## Quick Start (Run all commands in the root directory of the project)

### Running the proxy 

In a shell of your choice, while in the root directory of the project
```console
make clean                              # to clean any previous builds
make                                    # compiles the projet and produces an executable
./proxy                                 # runs the executable
```
### Launching the Management Console Web App

Set-up: On a separate shell tab/window, You only need to do this once.
```console
python -m venv venv                     # creates a virtual environment.
source venv/bin/activate                # activates virtual environment.
pip install -r requirements.txt         # installs all the requirements into the virtual environment.
```

After running the proxy, on a separate shell tab/window.Launch the web app (currently binds to localhost:3000)
```console
source venv/bin/activate                # activates virtual environment
python management_console.py            # run the Web App
```


## Project Organisation

/proxy_server  
├── Makefile  
├── README.md  
├── block_list.txt  
├── include  
│   ├── cache.h  
│   ├── console.h  
│   ├── http_handler.h  
│   ├── logging.h  
│   ├── management_console.h  
│   ├── proxy.h  
│   └── thread_pool.h  
├── management_console.py  
├── requirements.txt  
├── src  
│   ├── cache.c  
│   ├── console.c  
│   ├── http_handler.c  
│   ├── logging.c  
│   ├── main.c  
│   ├── management_console.c  
│   ├── proxy.c  
│   └── thread_pool.c  
└── tests
