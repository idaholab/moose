from PySide import *

def peacockError(moose_object, *args):
  moose_object._error_message.showMessage(" ".join(args))
