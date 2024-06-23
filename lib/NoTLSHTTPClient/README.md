An HTTPClient from Arduino with all TLS-related stull removed.
It can only connect via plan http and refuses all https URLs.
This allows to exclude mbedTLS code and certs bundle from firmware image and reduce fw image for about ~100k.
