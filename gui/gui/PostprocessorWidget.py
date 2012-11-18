#!usr/bin/python

import sys, os, random
from PyQt4 import QtGui, QtCore
import numpy, csv
from Plotter import *
from FlowLayout import *
import time


class PostprocessorWidget(QtGui.QWidget):
  '''
  the post-processor widget
  '''
  def __init__(self, input_file_widget, execute_widget):
    QtGui.QWidget.__init__(self)
    self.setAttribute(QtCore.Qt.WA_DeleteOnClose)
    self.input_file_widget = input_file_widget
    self.execute_widget = execute_widget
    
    self.currentFile = None
    self.getFileName()
    
    #Create the post processor list view
    self.postProcessorListView = QtGui.QListView()
    self.postProcessorListModel = QtGui.QStandardItemModel()
    self.postProcessorListView.setModel(self.postProcessorListModel)
    self.postProcessorModelFill()
   
    # adds a scroll layout to the main widget so all of the plots don't take up too much space       
    # scroll area widget contents - layout                                                                                                                        
    scroll = ResizeScrollArea()                                                                      
    wrapper = QtGui.QWidget()                                                                        
    self.flowLayout = FlowLayout()                                                                   
    wrapper.setLayout(self.flowLayout)                                                               
    scroll.setWidget(wrapper)                                                                        
    scroll.setWidgetResizable(True)                                                                                                                                   
    pal = QtGui.QPalette(scroll.palette())                                                           
    pal.setColor(QtGui.QPalette.Window, QtGui.QColor('lightgray'))                                   
    scroll.setPalette(pal)

    #assemble the top layout                                                                                         
    self.topLayout = QtGui.QGridLayout()
    self.topLayout.setColumnStretch (1, 2)
    self.topLayout.addWidget(self.postProcessorListView,0,0)
    self.topLayout.addWidget(scroll,0,1)

    # adds a button to the widget that will be used to clear plot selections
    self.clearButton = QtGui.QPushButton("Clear")
    self.clearButton.setToolTip('Clear current plots')
    self.clearButton.resize(self.clearButton.sizeHint())
    self.clearButton.clicked.connect(self.clearClick)

    # adds an open box to the widget
    self.openButton = QtGui.QPushButton("Open")
    self.openButton.setToolTip('Select an existing CSV file to read so the postprocessor values can be plotted')
    self.openButton.resize(self.clearButton.sizeHint())
    self.openButton.clicked.connect(self.openClick)

    #assemble the bottom layout
    self.buttonHBox = QtGui.QHBoxLayout()
    self.buttonHBox.addWidget(self.openButton)
    self.buttonHBox.addWidget(self.clearButton)
                                                                         
    #assemble the overall layout
    self.mainLayout = QtGui.QVBoxLayout()
    self.mainLayout.addLayout(self.topLayout)
    self.mainLayout.addLayout(self.buttonHBox)
    self.setLayout(self.mainLayout)
 
    self.timerSetUp()    

    self.executeSinglasLinking()        

    self.modifyUI()
    ''' This will be called after the interface is completely setup to allow an application to modify this tab '''
  def modifyUI(self):
      pass
      
      
  ''' Return the name to use for this tab '''
  def name(self):
      return 'Postprocess'
  
  def getFileName(self):
    '''
    get the file name of the csv file
    '''
    output_item = self.input_file_widget.tree_widget.findItems("Output", QtCore.Qt.MatchExactly)[0]
    cwd = str(self.execute_widget.cwd_text.text())
    self.currentFile = cwd +'/peacock_run_tmp_out.csv'
    if output_item:
      outputTable_data = output_item.table_data
      if 'file_base' in outputTable_data:
        self.currentFile = cwd +'/'+ outputTable_data['file_base'] + '.csv'
          
  def postProcessorModelFill(self):
    '''
    clean (if needed) and fill  the postProcessorListModel
    '''
    self.plotObjectDict  = {} #kept updated by addPlot
    self.plotDataDict    = {} #kept updated by addPlot
    self.plotHandlerDict = {} #kept updated by addPlot
    if self.currentFile and os.path.exists(self.currentFile):
      if os.path.getsize(self.currentFile) > 0: 
        self.postProcessorListModel.clear()
        try:
          self.postProcessorListModel.itemChanged.disconnect(self.itemChangedOnPostProcessorList)
        except:
          pass
        self.names = open(self.currentFile,'rb').readline()
        self.names = self.names.strip()
        self.names = self.names.split(',')
        for index in range(len(self.names)-1):
          item = QtGui.QStandardItem(self.names[index+1])
          item.setFlags(item.flags()|QtCore.Qt.ItemIsUserCheckable|QtCore.Qt.ItemIsEnabled)
          item.setData(QtCore.QVariant(QtCore.Qt.Checked), QtCore.Qt.CheckStateRole) #add the QtCore.Qt.Checked data (in a q form) to _item and set the QtCore.Qt.CheckStateRole as visualizzation rule
          item.setEditable(False)
          item.setCheckState(QtCore.Qt.Unchecked)
          self.postProcessorListModel.appendRow(item)
        self.postProcessorListModel.itemChanged.connect(self.itemChangedOnPostProcessorList)
      
  def itemChangedOnPostProcessorList(self,item):
    '''
    retrieve which item was changed and if present delete it otherwise add it
    '''
    text = str(item.text())
    if text in self.plotObjectDict.keys():
      self.plotHandlerDict[text].close()
      self.flowLayout.removeWidget(self.plotHandlerDict[text])
      del self.plotObjectDict[text]
      del self.plotDataDict[text]
      del self.plotHandlerDict[text]
    else:
      self.addPlot(text)

    
  
  def addPlot(self, plotName):
    # when this method is called a new item is added to the plots dictionary
    # then the getPlotData function is called to begin the first plot
#    print('adding plot ',str(plotName))
    time = [0]
    postData = [0]
    plotData = [time, postData]
    self.plotObjectDict[plotName] = MPLPlotter(plotData,plotName)
    self.plotDataDict[plotName] = [plotData]
    self.flowLayout.addWidget(self.plotObjectDict[plotName])
    self.plotHandlerDict[plotName] = self.flowLayout.itemAt(self.flowLayout.count()-1).widget()
    self.updatePlots()
  
                    
  def updatePlots(self):
      # this method loops through all of the plot objects in the plot dictionary
      # and for each plot calls the getPlotData function to update every plot that has been selected
#      print('updating plots')
      if self.currentFile and os.path.exists(self.currentFile):
          if os.path.getsize(self.currentFile) > 0:
              self.data = numpy.genfromtxt(self.currentFile,delimiter = ',' , names = True , invalid_raise = False)
              self.time = []

              try:
                  if self.data != None and len(self.data):        
                      for line in self.data:

                          self.time.append(line[0])

                      for key in self.plotObjectDict.keys():
                          self.postData = []
                          self.getPlotData(key)
                          plotData = [self.time, self.postData]
                          self.plotDataDict[key] = plotData

                      for key in self.plotDataDict:
                          self.plotObjectDict[key].setPlotData(self.plotDataDict[key],key)
              except:
                  pass

  
  def getPlotData(self, plotName):
#    print('getting plot data for '+str(plotName))
    indexCol = self.names.index(plotName)
    for line in self.data:
        self.postData.append(line[indexCol])
  

  def flushPlots(self):
    '''
    clean the flowLayout
    '''
#    print('flushing')
    i=0
    activeplot = self.flowLayout.count()
    while i < activeplot:
      self.flowLayout.itemAt(0).widget().close()
      self.flowLayout.removeWidget(self.flowLayout.itemAt(0).widget())
      i +=1
    
  
  def rePlot(self):
    '''
    collect the active plot rebuild the list and reactivate same name plot
    '''
#    print('replotting')
    cliked = self.plotHandlerDict.keys()
    self.postProcessorModelFill()
    for text in cliked:
      try:
        self.postProcessorListModel.findItems(text)[0].setCheckState(QtCore.Qt.Checked) #assuming we do not have more than one PP with the same name
      except:
        pass

  
  def openClick(self):
    '''
    open a new csv file, clean and rebuild Postprocessor list
    '''
    file_name = QtGui.QFileDialog.getOpenFileName(self, "Open CSV File", "~/", "CSV Files (*.csv)")
    if file_name:
      self.flushPlots()
      self.rePlot()

       
  def clearClick(self):
    '''
    retrive again the file name and rebuild 
    '''
    self.getFileName()
    self.flushPlots()
    self.postProcessorModelFill()
    
  def runClicked(self):
    self.getFileName()
    self.flushPlots()
    self.rePlot()

    
  def timerSetUp(self):
    # start and connect the time controls for updating the plots
    self.timer = QtCore.QTimer()
    self.timer.stop()
    self.timer.setInterval(1000)
    self.timer.timeout.connect(self.updatePlots)
    
  def executeSinglasLinking(self):
    #set up the signals from the simulation 
    self.execute_widget.run_button.clicked.connect(self.runClicked)
    self.execute_widget.run_button.clicked.connect(self.timer.start)
    self.execute_widget.run_stopped.connect(self.updatePlots)
    self.execute_widget.run_stopped.connect(self.timer.stop)
   



    
class ResizeScrollArea(QtGui.QScrollArea):
  
  def __init(self, parent=None):  
    QtGui.QScrollArea.__init__(self, parent)
  
  def resizeEvent(self, event):
    wrapper = self.findChild(QtGui.QWidget)
    flow = wrapper.findChild(FlowLayout)
    
    width = self.viewport().width()
    height = flow.heightForWidth(width)
    size = QtCore.QSize(width, height)
    point = self.viewport().rect().topLeft()
    
    flow.setGeometry(QtCore.QRect(point, size))
    
    self.viewport().update()  
    QtGui.QScrollArea.resizeEvent(self, event)
