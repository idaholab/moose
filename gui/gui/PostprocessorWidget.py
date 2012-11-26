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
    
    self.current_file = None
    self.getFileName()
    
    #Create the post processor list view
    self.post_processor_list_view = QtGui.QListView()
    self.post_processor_list_model = QtGui.QStandardItemModel()
    self.post_processor_list_view.setModel(self.post_processor_list_model)
    self.postProcessorModelFill()
   
    # adds a scroll layout to the main widget so all of the plots don't take up too much space       
    # scroll area widget contents - layout                                                                                                                        
    scroll = ResizeScrollArea()                                                                      
    wrapper = QtGui.QWidget()                                                                        
    self.flow_layout = FlowLayout()
    wrapper.setLayout(self.flow_layout)                                                               
    scroll.setWidget(wrapper)                                                                        
    scroll.setWidgetResizable(True)                                                                                                                                   
    pal = QtGui.QPalette(scroll.palette())                                                           
    pal.setColor(QtGui.QPalette.Window, QtGui.QColor('lightgray'))                                   
    scroll.setPalette(pal)

    # adds a button to the widget that will be used to clear plot selections
    self.clear_button = QtGui.QPushButton("Clear")
    self.clear_button.setToolTip('Clear current plots')
    self.clear_button.resize(self.clear_button.sizeHint())
    self.clear_button.clicked.connect(self.clearClick)

    # adds an open box to the widget
    self.open_button = QtGui.QPushButton("Open")
    self.open_button.setToolTip('Select an existing CSV file to read so the postprocessor values can be plotted')
    self.open_button.resize(self.clear_button.sizeHint())
    self.open_button.clicked.connect(self.openClick)

    #assemble the bottom layout
    self.button_layout = QtGui.QHBoxLayout()
    self.button_layout.addWidget(self.open_button)
    self.button_layout.addWidget(self.clear_button)

    #assemble the top layout
    self.left_widget = QtGui.QWidget()
    self.left_layout = QtGui.QVBoxLayout()
    self.left_widget.setLayout(self.left_layout)
    
    self.left_layout.addWidget(self.post_processor_list_view)
    self.left_layout.addLayout(self.button_layout)
    
    self.top_layout = QtGui.QSplitter()
    self.top_layout.addWidget(self.left_widget)
    self.top_layout.addWidget(scroll)

    self.top_layout.setStretchFactor(0,0.1)
    self.top_layout.setStretchFactor(1,0.9)

    self.top_layout.setSizes([100,500])    
                                                                         
    #assemble the overall layout
    self.main_layout = QtGui.QVBoxLayout()
    self.main_layout.addWidget(self.top_layout)
    self.setLayout(self.main_layout)
 
    self.timerSetUp()    

    self.executeSignalLinking()        

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
    self.current_file = cwd +'/peacock_run_tmp_out.csv'
    if output_item:
      outputTable_data = output_item.table_data
      if 'file_base' in outputTable_data:
        self.current_file = cwd +'/'+ outputTable_data['file_base'] + '.csv'
          
  def postProcessorModelFill(self):
    '''
    clean (if needed) and fill  the post_processor_list_model
    '''
    self.plot_objects  = {} #kept updated by addPlot
    self.plot_datas    = {} #kept updated by addPlot
    self.plot_handlers = {} #kept updated by addPlot
    if self.current_file and os.path.exists(self.current_file):
      if os.path.getsize(self.current_file) > 0: 
        self.post_processor_list_model.clear()
        try:
          self.post_processor_list_model.itemChanged.disconnect(self.itemChangedOnPostProcessorList)
        except:
          pass
        self.names = open(self.current_file,'rb').readline()
        self.names = self.names.strip()
        self.names = self.names.split(',')
        for index in range(len(self.names)-1):
          item = QtGui.QStandardItem(self.names[index+1])
          item.setFlags(item.flags()|QtCore.Qt.ItemIsUserCheckable|QtCore.Qt.ItemIsEnabled)
          item.setData(QtCore.QVariant(QtCore.Qt.Checked), QtCore.Qt.CheckStateRole) #add the QtCore.Qt.Checked data (in a q form) to _item and set the QtCore.Qt.CheckStateRole as visualizzation rule
          item.setEditable(False)
          item.setCheckState(QtCore.Qt.Unchecked)
          self.post_processor_list_model.appendRow(item)
        self.post_processor_list_model.itemChanged.connect(self.itemChangedOnPostProcessorList)
      
  def itemChangedOnPostProcessorList(self,item):
    '''
    retrieve which item was changed and if present delete it otherwise add it
    '''
    text = str(item.text())
    if text in self.plot_objects.keys():
      self.plot_handlers[text].close()
      self.flow_layout.removeWidget(self.plot_handlers[text])
      del self.plot_objects[text]
      del self.plot_datas[text]
      del self.plot_handlers[text]
    else:
      self.addPlot(text)

    
  
  def addPlot(self, plotName):
    # when this method is called a new item is added to the plots dictionary
    # then the getPlotData function is called to begin the first plot
#    print('adding plot ',str(plotName))
    time = [0]
    post_data = [0]
    plot_data = [time, post_data]
    self.plot_objects[plotName] = MPLPlotter(plot_data,plotName)
    self.plot_datas[plotName] = [plot_data]
    self.flow_layout.addWidget(self.plot_objects[plotName])
    self.plot_handlers[plotName] = self.flow_layout.itemAt(self.flow_layout.count()-1).widget()
    self.updatePlots()
  
                    
  def updatePlots(self):
      # this method loops through all of the plot objects in the plot dictionary
      # and for each plot calls the getPlotData function to update every plot that has been selected
#      print('updating plots')
      if self.current_file and os.path.exists(self.current_file):
          if os.path.getsize(self.current_file) > 0:
              self.data = numpy.genfromtxt(self.current_file,delimiter = ',' , names = True , invalid_raise = False)
              self.time = []

              try:
                  if self.data != None and len(self.data):        
                      for line in self.data:

                          self.time.append(line[0])

                      for key in self.plot_objects.keys():
                          self.post_data = []
                          self.getPlotData(key)
                          plot_data = [self.time, self.post_data]
                          self.plot_datas[key] = plot_data

                      for key in self.plot_datas:
                          self.plot_objects[key].setPlotData(self.plot_datas[key],key)
              except:
                  pass

  
  def getPlotData(self, plotName):
#    print('getting plot data for '+str(plotName))
    index_col = self.names.index(plotName)
    for line in self.data:
        self.post_data.append(line[index_col])
  

  def flushPlots(self):
    '''
    clean the flow_layout
    '''
#    print('flushing')


    i=0
    active_plot = self.flow_layout.count()
    while i < active_plot:
      self.flow_layout.itemAt(0).widget().close()
      self.flow_layout.removeWidget(self.flow_layout.itemAt(0).widget())
      i +=1
    
  
  def rePlot(self):
    '''
    collect the active plot rebuild the list and reactivate same name plot
    '''
#    print('replotting')
    cliked = self.plot_handlers.keys()
    self.postProcessorModelFill()
    for text in cliked:
      try:
        self.post_processor_list_model.findItems(text)[0].setCheckState(QtCore.Qt.Checked) #assuming we do not have more than one PP with the same name
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
    self.flushPlots()
    self.getFileName()
    self.postProcessorModelFill()
    
  def runClicked(self):
    self.flushPlots()
    self.getFileName()
    self.rePlot()
    
  def timerSetUp(self):
    # start and connect the time controls for updating the plots
    self.timer = QtCore.QTimer()
    self.timer.stop()
    self.timer.setInterval(1000)
    self.timer.timeout.connect(self.updatePlots)

  ''' Here to provide a buffer for run stoppage '''
  def runStoppedExecute(self):
    self.flushPlots()
    self.rePlot()
    self.updatePlots()

  def runStopped(self):
    self.timer.stop
    self.run_stopped_timer = QtCore.QTimer()
    self.run_stopped_timer.setInterval(1000) # Wait a second before updating the plots one last time
    self.run_stopped_timer.setSingleShot(True)
    self.run_stopped_timer.timeout.connect(self.runStoppedExecute)
    self.run_stopped_timer.start()
    
  def executeSignalLinking(self):
    #set up the signals from the simulation 
    self.execute_widget.run_button.clicked.connect(self.runClicked)
    self.execute_widget.run_button.clicked.connect(self.timer.start)
    self.execute_widget.run_stopped.connect(self.runStopped)
    
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
