from RunApp import RunApp
from util import runCommand
import os
import web, threading

urls = ('/ice/update', 'ICEUpdaterTester')

class ICEUpdaterTester(RunApp, web.application):

  postCounter = 0
  
  @staticmethod
  def validParams():
    params = RunApp.validParams()
    params.addRequiredParam('nPosts', "The Number of Expected Posts")
    return params
        
  def __init__(self, name=None, params=None):
    RunApp.__init__(self, name, params)
    web.application.__init__(self, urls, globals())
    self.httpThread = threading.Thread(target=self.run)
    ICEUpdaterTester.postCounter += 1
    if params is not None:
       self.nPosts = self.getParam('nPosts')

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
    if ICEUpdaterTester.postCounter != int(self.nPosts):
       return ("DID NOT GET CORRECT NUMBER OF POSTS", False)
    else:
       return RunApp.processResults(self, moose_dir, retcode, options, output)
  
  def run(self, port=8080, *middleware):
    func = self.wsgifunc(*middleware)
    return web.httpserver.runsimple(func, ('127.0.0.1', port))

  def POST(self):
    data = web.data()
    print data