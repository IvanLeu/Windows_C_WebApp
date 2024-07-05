#pragma once

#define OK_RESPONSE "HTTP/1.0 200 OK\r\n\r\n"
#define OK_RESPONSE_HTML "HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n"
#define NOT_FOUND_RESPONSE "HTTP/1.1 404 NOT FOUND\r\n\r\n"
#define TEMPORARY_REDIRECT "HTTP/1.1 302 Found\r\nLocation: %s\r\n\r\n"

// each call need to pass the content length (file size)
#define DOWNLOAD_FILE_RESPONSE "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Disposition: attachment; filename=\"index.html\"\r\nContent-Length: %ld\r\n\r\n"