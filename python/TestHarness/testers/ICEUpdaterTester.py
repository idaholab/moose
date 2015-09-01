from RunApp import RunApp
from util import runCommand
import os
import web, threading

urls = ('/ice/update', 'ice_update')

class ice_update:
    def POST(self):
      print 'HELLO'
      data = web.data()
      print data

class ICEUpdaterTester(RunApp, web.application):
    
  @staticmethod
  def validParams():
    params = RunApp.validParams()

    return params
        
  def __init__(self, name, params):
    RunApp.__init__(self, name, params)
    web.application.__init__(self, urls, globals())
    self.httpThread = threading.Thread(target=self.run)
  
  # This method is called prior to running the test.  It can be used to cleanup files
  # or do other preparations before the tester is run
  def prepare(self):
    return
    
  # This method is called to return the commands (list) used for processing results
  def processResultsCommand(self, moose_dir, options):
    commands = []
    return commands

  # This method should return the executable command that will be executed by the tester
  def getCommand(self, options):
    command = RunApp.getCommand(self, options)
    self.httpThread.start()
    return command

  # This method will be called to process the results of running the test.  Any post-test
  # processing should happen in this method
  def processResults(self, moose_dir, retcode, options, output):
    self.stop()
    return RunApp.processResults(self, moose_dir, retcode, options, output)

  def run(self, port=8080, *middleware):
    func = self.wsgifunc(*middleware)
    return web.httpserver.runsimple(func, ('127.0.0.1', port))
