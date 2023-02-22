import requests
import time

# Set address and port
moose_url = "http://localhost:8000"

# Check and see if MOOSE is waiting
def isWaiting():
    response = requests.get(moose_url + "/waiting")
    return response.json()["waiting"]

# Wait for MOOSE to be waiting
def waitForMOOSE():
    waiting = isWaiting()
    while not waiting:
        time.sleep(1)
        waiting = isWaiting()

# Get postprocessor value
def getPostprocessorValue(pp_name):
    if isWaiting():
        response = requests.post(moose_url + "/get_pp", json = {"pp": pp_name})
        return response.json()["value"]
    else:
        return {}

# Set the controlled value
def setControllable(value):
    if isWaiting():
        response = requests.post(moose_url + "/set_controllable", json = {"value": value})
        return True
    else:
        return False

# Tell MOOSE to continue
def continueSolve():
    if isWaiting():
        response = requests.get(moose_url + "/continue")
        time.sleep(0.1) # Wait for a moment to make sure this has occurred
        return True
    else:
        return False

# Wait for MOOSE to start up
time.sleep(10)

# on_initial
waitForMOOSE()
setControllable(0)
getPostprocessorValue("coef")
continueSolve()

# Each on_timestep_begin
for i in range(0, 10):
    waitForMOOSE()
    setControllable(i)
    getPostprocessorValue("coef")
    continueSolve()
