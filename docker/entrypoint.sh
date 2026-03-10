#!/bin/bash
cd /app
chown -R 1000:1000 /app
chmod -R 755 /app

su server -c "$@"
