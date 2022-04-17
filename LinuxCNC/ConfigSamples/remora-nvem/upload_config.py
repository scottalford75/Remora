#!/usr/bin/env python3

import binascii
import sys
import math
import json
import tftpy
import time


if len(sys.argv) < 2:
    print("usage: upload.py <config.txt>")
    exit(-1)


# Check for a valid JSON formatted file, if ok upload to NVEM board via TFTP
with open(sys.argv[1], 'r') as jsonFile:
    try:
        testForValidJson = json.load(jsonFile)
        print('Valid JSON config file, uploading to NVEM board')
                
    except ValueError:
        print('Incorrectly formatted JSON configuration file')
        exit(-1)


# Load File
config = open(sys.argv[1], "rb").read()

# Compute length (in bytes and words)
jsonLength = len(config)
length = math.ceil(jsonLength / 4)
mod = jsonLength % 4
print('Config file length (words) =', length)
print('Config file length (bytes) =', jsonLength)
print('Remainder =', mod)

# add padding at end of file to ensure length is multiple of 4 bytes (word)
# to ensure correct CRC32 calculation
if mod > 0:
    padding = [0 for i in range(4 - mod)]
    print('Padding added = ', padding)
    config = config + bytes(padding)

newLength = len(config)
print('Config file length with padding (bytes) =', newLength)

# Compute crc32
crc32 = binascii.crc32(config)
print('CRC-32 =',hex(crc32))

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
metadata[8] = jsonLength & 0xff
metadata[9] = (jsonLength >> 8) & 0xff
metadata[10] = (jsonLength >> 16) & 0xff
metadata[11] = (jsonLength >> 24) & 0xff
config = bytes(metadata) + config

open("/tmp/config.txt", "wb").write(config)

# Upload using TFTP, set large timeout
client = tftpy.TftpClient("10.10.10.10", 69)
client.upload("config", "/tmp/config.txt", timeout=30)
