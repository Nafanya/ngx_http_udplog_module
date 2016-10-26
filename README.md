# Nginx udplog module
This module provides simple UDP logging of incoming HTTP requests' uris (`ngx_http_request_t.uri`) and their CRC-32 checksums for nginx web server.

Tested with mainline nginx version 1.11.5.

## Description
Packets are sent over UDP during log phase (`NGX_HTTP_LOG_PHASE`) since it's made specially for such kind of things.
There is one memory allocation per incoming request to include uri's CRC-32 checksum. It possibly can be avoided at the cost of destructive changes of urs's `data` field. To achieve this we need to write extra 4 bytes for CRC-32, so uri itself can be compressed somehow to become 4 bytes less, but it barely works for short uris (e.g. uri `/` is 1 byte long). Another possible approach is to preallocate enough buffers and use them to serve requests.

# Configuration directives

syntax: `udplog <hostname> <port>`

context: `main`

# Example configuration

```
http {

    [...]
    
    udplog 127.0.0.1 9999;

    server {
        [...]
    }
}
```

# Testing

This module can be tested in two ways: with `netcat` utility or with `server.py`.

## netcat

To test module locally, use:

`$ netcat -ul PORT`

e.g. `$ netcat -ul 9999` to listen on localhost

## server.py

This is a python3 script, adjust `HOST` and `PORT` variables in it and you are ready to go. Defaults are localhost and port 9999.
