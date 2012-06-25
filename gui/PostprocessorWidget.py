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
        
        self.getFileName()
        
        # uses the postprocessor selected by the user to pass the information required for plotting
        self.comboWidget = QtGui.QComboBox(self)
        self.fillComboWidget()
        self.comboWidget.activated[str].connect(self.createPlot)
        self.plotObjectDict = {}
        self.plotDataDict = {}
        
        # adds a button to the widget that will be used to clear plot selectons
        self.clearButton = QtGui.QPushButton("Clear")
        self.clearButton.resize(self.clearButton.sizeHint())
        self.clearButton.clicked.connect(self.clearClick)

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
        QtCore.QObject.connect(self.execute_widget.run_button, QtCore.SIGNAL("clicked()"), self.runClicked)
        QtCore.QObject.connect(self.timer, QtCore.SIGNAL("timeout()"), self.updatePlots)
        self.timer.start(500)
        
        
        # sets the layout for the widget
        self.Layout = QtGui.QVBoxLayout(self)
        self.Layout.addWidget(self.comboWidget)
        self.Layout.addWidget(self.scrollArea)
        self.Layout.addWidget(self.clearButton)

   
    
    def getFileName(self):
        
        output_item = self.input_file_widget.tree_widget.findItems("Output", QtCore.Qt.MatchExactly)[0]
        self.cwd = os.getcwd()
        self.currentFile = self.cwd +'/peacock_run_tmp_out.csv'
    
        if output_item:
            outputTable_data = output_item.table_data
            
            if 'file_base' in outputTable_data:
                self.currentFile = self.cwd +'/'+ outputTable_data['file_base'] + '.csv'
            
    
    def fillComboWidget(self):
        
        if os.path.exists(self.currentFile):
        
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
        
        
        # adds a layout to the PPD plot
        self.scrollLayout.addRow(self.plotObjectDict[plotName])
        self.updatePlots()
    
    def updatePlots(self):
        # this method loops through all of the plot objects in the plot dictionary
        # and for each plot calls the getPlotData function to update every plot that has been selected
        
        
        if os.path.exists(self.currentFile):
            self.data = numpy.genfromtxt(self.currentFile,
                                delimiter = ',' , names = True)
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
        self.clearClick()

    def clearClick(self):
        for i in range(self.scrollLayout.count()):
            self.scrollLayout.itemAt(i).widget().close()
        self.plotObjectDict = {}
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
