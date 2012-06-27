#!usr/bin/python

import sys, os, random
from PyQt4 import QtGui, QtCore
import numpy, csv
from Plotter import *



class PostprocessorWidget(QtGui.QWidget):
    def __init__(self, input_file_widget, execute_widget):
        QtGui.QWidget.__init__(self)
        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)
        self.input_file_widget = input_file_widget
        self.execute_widget = execute_widget
        
        self.currentFile = None
        
        # uses the postprocessor selected by the user to pass the information required for plotting
        
        self.cwd = os.getcwd()
        self.comboWidget = QtGui.QComboBox(self)
        self.fillComboWidget()
        self.comboWidget.activated[str].connect(self.createPlot)
        self.plotObjectDict = {}
        self.plotDataDict = {}
        self.counter = 0
        self.plotIndex = 0
        
        
        
        # adds a button to the widget that will be used to clear plot selectons
        self.clearButton = QtGui.QPushButton("Clear")
        self.clearButton.resize(self.clearButton.sizeHint())
        self.clearButton.clicked.connect(self.clearClick)
        # adds an open box to the widget
        self.openButton = QtGui.QPushButton("Open")
        self.openButton.resize(self.clearButton.sizeHint())
        self.openButton.clicked.connect(self.openClick)
            
        self.mainHBox = QtGui.QHBoxLayout()
        self.mainHBox.addWidget(self.openButton)
        self.mainHBox.addWidget(self.clearButton)

        # adds a scroll layout to the main widget so all of the plots don't take up too much space
        # scroll area widget contents - layout
        self.scrollLayout = QtGui.QFormLayout()
        self.scrollWidget = QtGui.QWidget()
        self.scrollWidget.setLayout(self.scrollLayout)
        self.scrollArea = QtGui.QScrollArea()
        self.scrollArea.setWidgetResizable(True)
        self.scrollArea.setHorizontalScrollBarPolicy(1)
        self.scrollArea.setWidget(self.scrollWidget)
        
        
        # timer feature that updates all of the plots
        self.timer = QtCore.QTimer(self)
        self.timer.stop()
        self.timer2 = QtCore.QTimer(self)
        QtCore.QObject.connect(self.execute_widget.run_button, QtCore.SIGNAL("clicked()"), self.runClicked)
        QtCore.QObject.connect(self.timer, QtCore.SIGNAL("timeout()"), self.updatePlots)        
        
        # sets the layout for the widget
        self.Layout = QtGui.QVBoxLayout(self)
        self.Layout.addWidget(self.comboWidget)
        self.Layout.addWidget(self.scrollArea)
        self.Layout.addLayout(self.mainHBox)

   
    
    def getFileName(self):
        
        output_item = self.input_file_widget.tree_widget.findItems("Output", QtCore.Qt.MatchExactly)[0]

        self.currentFile = self.cwd +'/peacock_run_tmp_out.csv'
    
        if output_item:
            outputTable_data = output_item.table_data
            
            if 'file_base' in outputTable_data:
                self.currentFile = self.cwd +'/'+ outputTable_data['file_base'] + '.csv'
            
    
    def fillComboWidget(self):
        
        if self.currentFile and os.path.exists(self.currentFile):
            if os.path.getsize(self.currentFile) > 0:
        
                self.names = open(self.currentFile,'rb').readline()
                self.names = self.names.strip()
                self.names = self.names.split(',')
            
                    # the PP names are used to populate a dropdown menu 
                    # the user uses  this menu to select specific graphs to be monitored
                for index in range(len(self.names)-1):
                    self.comboWidget.addItem(self.names[index+1])
        
                self.comboWidget.setCurrentIndex(-1)
            else:
                self.timer2.start(1000)
        
    
    
    def createPlot(self, plotName):
        
        # when this method is called a new item is added to the plots dictionary
        # then the getPlotData function is called to begin the first plot
        self.timer.start(500)
        time = [0]
        postData = [0]
        plotData = [time, postData]
        self.plotObjectDict[plotName] = MPLPlotter(plotData,plotName)
        self.plotDataDict[plotName] = [plotData]
        self.hbox = QtGui.QHBoxLayout()

        
        if (self.counter == 0):
            self.hbox.addWidget(self.plotObjectDict[plotName])
            # adds a layout to the PPD plot
            self.scrollLayout.addRow(self.hbox)
            self.counter = 1
        else:
            self.scrollLayout.itemAt(self.plotIndex).addWidget(self.plotObjectDict[plotName])
            self.plotIndex += 1
            self.counter = 0


        self.updatePlots()
    
    def updatePlots(self):
        # this method loops through all of the plot objects in the plot dictionary
        # and for each plot calls the getPlotData function to update every plot that has been selected
        
        
        if self.currentFile and os.path.exists(self.currentFile):
            if os.path.getsize(self.currentFile) > 0:
                self.data = numpy.genfromtxt(self.currentFile,delimiter = ',' , names = True)
                self.time = []
        
                for line in self.data:

                    self.time.append(line[0])
            
                for key in self.plotObjectDict.keys():
                    self.postData = []
                    self.getPlotData(key)
                    plotData = [self.time, self.postData]
                    self.plotDataDict[key] = plotData
    
                for key in self.plotDataDict:
                        self.plotObjectDict[key].setPlotData(self.plotDataDict[key],key)
                    


    
    def getPlotData(self, plotName):
        
       
        indexCol = self.names.index(plotName)
        for line in self.data:
            self.postData.append(line[indexCol])
    
    def runClicked(self):
        QtCore.QObject.connect(self.execute_widget.proc,QtCore.SIGNAL("finished(int)"),self.stopPlotting)
        
        QtCore.QObject.connect(self.timer2, QtCore.SIGNAL("timeout()"), self.clearClick)
        self.timer2.start(1000)
    
    
    def openClick(self):
        
        file_name = QtGui.QFileDialog.getOpenFileName(self, "Open CSV File", "~/", "CSV Files (*.csv)")
        if file_name:
            self.currentFile = str(file_name)
            self.timer2.stop()
            self.timer.stop()
            for i in range(self.scrollLayout.count()):
                for j in range(self.scrollLayout.itemAt(i).count()):
                    self.scrollLayout.itemAt(i).itemAt(j).widget().close()
                self.scrollLayout.itemAt(i).removeItem(self.hbox)
            self.plotObjectDict = {}
            self.counter = 0
            self.plotDataDict = {}
            self.comboWidget.clear()
            self.fillComboWidget()
            self.comboWidget.setCurrentIndex(-1)
    
    def clearClick(self):
        self.timer2.stop()
        for i in range(self.scrollLayout.count()):
            for j in range(self.scrollLayout.itemAt(i).count()):
                self.scrollLayout.itemAt(i).itemAt(j).widget().close()
            self.scrollLayout.itemAt(i).removeItem(self.hbox)
        self.plotObjectDict = {}
        self.counter = 0
        self.plotDataDict = {}
        self.comboWidget.clear()
        self.getFileName()
        self.fillComboWidget()
        self.comboWidget.setCurrentIndex(-1)
        

    def stopPlotting(self):
        
        self.comboWidget.clear()
        self.getFileName()
        self.fillComboWidget()
        self.updatePlots()
        self.timer.stop()
