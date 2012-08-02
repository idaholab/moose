#!usr/bin/python

import sys, os, random
from PyQt4 import QtGui, QtCore
import numpy, csv
from Plotter import *
from FlowLayout import *



class PostprocessorWidget(QtGui.QWidget):
    def __init__(self, input_file_widget, execute_widget):
        QtGui.QWidget.__init__(self)
        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)
        self.input_file_widget = input_file_widget
        self.execute_widget = execute_widget
        
        self.currentFile = None
        
        # uses the postprocessor selected by the user to pass the information required for plotting
        
        self.comboWidget = QtGui.QComboBox(self)
        self.comboWidget.setToolTip('Select a postprocessor from this list to add a plot for that postprocessor')
        self.fillComboWidget()
        self.comboWidget.activated[str].connect(self.createPlot)
        self.plotObjectDict = {}
        self.plotDataDict = {}

        # adds a button to the widget that will be used to clear plot selectons
        self.clearButton = QtGui.QPushButton("Clear")
        self.clearButton.setToolTip('Clear current plots')
        self.clearButton.resize(self.clearButton.sizeHint())
        self.clearButton.clicked.connect(self.clearClick)
        # adds an open box to the widget
        self.openButton = QtGui.QPushButton("Open")
        self.openButton.setToolTip('Select an existing CSV file to read so the postprocessor values can be plotted')
        self.openButton.resize(self.clearButton.sizeHint())
        self.openButton.clicked.connect(self.openClick)
            
        self.buttonHBox = QtGui.QHBoxLayout()
        self.buttonHBox.addWidget(self.openButton)
        self.buttonHBox.addWidget(self.clearButton)

        # adds a scroll layout to the main widget so all of the plots don't take up too much space
        # scroll area widget contents - layout
        self.plotHBox = QtGui.QHBoxLayout()
        scroll = ResizeScrollArea()
        wrapper = QtGui.QWidget()
        self.flowLayout = FlowLayout()
        wrapper.setLayout(self.flowLayout)
        scroll.setWidget(wrapper)
        scroll.setWidgetResizable(True)
        self.plotHBox.addWidget(scroll)
        pal = QtGui.QPalette(scroll.palette())
        pal.setColor(QtGui.QPalette.Window, QtGui.QColor('lightgray'))
        scroll.setPalette(pal)
        
        
        # timer feature that updates all of the plots
#        self.timer = QtCore.QTimer(self)
#        self.timer.stop()
#        self.timer2 = QtCore.QTimer(self)
#        QtCore.QObject.connect(self.timer2, QtCore.SIGNAL("timeout()"), self.fillComboWidget)
#        QtCore.QObject.connect(self.execute_widget.run_button, QtCore.SIGNAL("clicked()"), self.runClicked)
#        QtCore.QObject.connect(self.timer, QtCore.SIGNAL("timeout()"), self.updatePlots)

        self.execute_widget.run_started.connect(self.clearClick)
        self.execute_widget.timestep_end.connect(self.updatePlots)
        self.execute_widget.run_stopped.connect(self.updatePlots)

        
        # sets the layout for the widget
        self.Layout = QtGui.QVBoxLayout(self)
        self.Layout.addWidget(self.comboWidget)
        self.Layout.addLayout(self.plotHBox)
        self.Layout.addLayout(self.buttonHBox)
    
    def getFileName(self):
        
        output_item = self.input_file_widget.tree_widget.findItems("Output", QtCore.Qt.MatchExactly)[0]
        cwd = str(self.execute_widget.cwd_text.text())
        self.currentFile = cwd +'/peacock_run_tmp_out.csv'
    
        if output_item:
            outputTable_data = output_item.table_data
            
            if 'file_base' in outputTable_data:
                self.currentFile = cwd +'/'+ outputTable_data['file_base'] + '.csv'
            
    
    def fillComboWidget(self):
        
        if self.currentFile and os.path.exists(self.currentFile):
            if os.path.getsize(self.currentFile) > 0:
                
                self.comboWidget.clear()
        
                self.names = open(self.currentFile,'rb').readline()
                self.names = self.names.strip()
                self.names = self.names.split(',')
            
                    # the PP names are used to populate a dropdown menu 
                    # the user uses  this menu to select specific graphs to be monitored
                for index in range(len(self.names)-1):
                    self.comboWidget.addItem(self.names[index+1])
        
                self.comboWidget.setCurrentIndex(-1)
        
    
    def createPlot(self, plotName):
        # when this method is called a new item is added to the plots dictionary
        # then the getPlotData function is called to begin the first plot
        time = [0]
        postData = [0]
        plotData = [time, postData]
        self.plotObjectDict[plotName] = MPLPlotter(plotData,plotName)
        self.plotDataDict[plotName] = [plotData]
        self.flowLayout.addWidget(self.plotObjectDict[plotName])

        self.updatePlots()
    
    
    def updatePlots(self):
        # this method loops through all of the plot objects in the plot dictionary
        # and for each plot calls the getPlotData function to update every plot that has been selected
        if self.first:
            self.first = False
            self.fillComboWidget()
            self.comboWidget.setCurrentIndex(-1)
        
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
        indexCol = self.names.index(plotName)
        for line in self.data:
            self.postData.append(line[indexCol])
    
#    def runClicked(self):
#        QtCore.QObject.connect(self.execute_widget.proc,QtCore.SIGNAL("finished(int)"),self.stopPlotting)
        
#        QtCore.QObject.connect(self.timer2, QtCore.SIGNAL("timeout()"), self.clearClick)
#        self.timer2.start(1000)
    
    
    def openClick(self):
        self.first = True
        file_name = QtGui.QFileDialog.getOpenFileName(self, "Open CSV File", "~/", "CSV Files (*.csv)")
        if file_name:
            self.currentFile = str(file_name)
            for i in range(self.flowLayout.count()):
                self.flowLayout.itemAt(i).widget().close()
            self.plotObjectDict = {}
            self.counter = 0
            self.plotDataDict = {}
            self.comboWidget.clear()
            self.fillComboWidget()

    
    def clearClick(self):
        
        self.getFileName()
        self.first = True
        for i in range(self.flowLayout.count()):
            self.flowLayout.itemAt(i).widget().close()
        self.plotObjectDict = {}
        self.counter = 0
        self.plotDataDict = {}
        self.comboWidget.clear()
        self.fillComboWidget()

        
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
