#!usr/bin/python3

import random
import time
import pycurl
import json
from subprocess import Popen, PIPE

try:
    # python 3
    from urllib.parse import urlencode
    from io import BytesIO
except ImportError:
    # python 2
    from urllib import urlencode
    from StringIO import StringIO as BytesIO

# update interval in minutes this would be read from a config file
update_interval = 0.1

# The UUID is initialized from the device configuration
uuid = 'invalid'

# The idea is there will be more entry points for things like bee/camerafeed ..etc
base = 'http://localhost:3420/'
netupdate_url = base + 'bee/network'

# Read the UUID from the beemon file
beemon = open("beemon.cfg", 'r')
for line in beemon:
    if line[0] == '#': continue
    param = line.split('=')[0]
    value = line.split('=')[1]
    if param == "uuid":
        uuid = value.strip()
    elif param == "endpoint":
        base = value.strip()

print("UUID: " + uuid)
print("Endpoint: " + base)

wireless_interface = 'wlxacf1df10afba'

def update():
    c = pycurl.Curl()
    c.setopt(c.URL, netupdate_url)
    c.setopt(c.WRITEFUNCTION, buffer.write)

    available_ssids = []
    network_types = []

    # Get the available networks and send their SSID and type
    settingLine = True
    pipe = Popen("iwlist wlxacf1df10afba s | grep -E 'SSID|Encryption'", shell=True, stdout=PIPE)

    for index, line in enumerate(pipe.stdout):
        if not settingLine:
            ssid = line.strip().split(b':')[1][1:-1]
            if ssid: 
                available_ssids.append(ssid)
        else:
            network_types.append(line.strip().split(b':')[1])

        settingLine = not settingLine

    available_networks = []
    for index, ssid in enumerate(available_ssids):
        available_networks.append({
            'ssid' : ssid.decode('utf-8'),
            'type' : network_types[index].decode("utf-8")
        })

    post_data = {'uuid': uuid, 'networks': json.dumps(available_networks)}
    c.setopt(c.POSTFIELDS, urlencode(post_data))

    c.perform()
    http_response_code = c.getinfo(pycurl.HTTP_CODE)

    c.close()
    return http_response_code


# todo: not crash when no connection
while True:
    try:
        buffer = BytesIO()
        http_code = update()
        response = buffer.getvalue()

        print(response)
    except pycurl.error:
        print("Error contacting server!")
        continue
    
    time.sleep(update_interval*60)
