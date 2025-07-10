from MooseControl import MooseControl
import logging
from http.client import HTTPConnection

# HTTPConnection.debuglevel = 1
# requests_log = logging.getLogger("requests.packages.urllib3")
# requests_log.setLevel(logging.DEBUG)
# requests_log.propagate = True

control = MooseControl(moose_port=12345)
control.initialize()

control.wait('INITIAL')
control.setContinue()

for i in range(3):
  control.wait('TIMESTEP_BEGIN')
  t = control.getPostprocessor('t')
  print(f'the value of t is {t}')
  control.setContinue()

control.finalize()
