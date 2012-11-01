from options import *

class Tester(object):
#  def __new__(klass, type, specs):
#    return super(Tester, klass).__new__(type)

  def __init__(self, klass, specs):
    self.specs = specs

  def prepare(self):
    return

  def getCommand(self, options):
    return

  def processResults(self, moose_dir, retcode, output):
    return

