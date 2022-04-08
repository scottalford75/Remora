#!/usr/bin/env python3

import zlib
import sys
import tftpy
import time

if len(sys.argv) < 2:
    print("usage: upload.py <config.txt>")
    exit(-1)

# Load File
config = open(sys.argv[1], "rb").read()

# Compute length (in bytes)
length = len(config)

# Compute crc32
crc32 = zlib.crc32(config)


# Insert metadata
metadata = [0 for i in range(512)]
metadata[0] = crc32 & 0xff
metadata[1] = (crc32 >> 8) & 0xff
metadata[2] = (crc32 >> 16) & 0xff
metadata[3] = (crc32 >> 24) & 0xff
metadata[4] = length & 0xff
metadata[5] = (length >> 8) & 0xff
metadata[6] = (length >> 16) & 0xff
metadata[7] = (length >> 24) & 0xff
config = bytes(metadata) + config

open("/tmp/config.txt", "wb").write(config)

# Upload using TFTP, set large timeout
client = tftpy.TftpClient("10.10.10.10", 69)
client.upload("config", "/tmp/config.txt", timeout=30)

