#!usr/bin/python

import sys, os, random
from PyQt4 import QtGui, QtCore
import numpy, csv
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure


class PlotWidget(FigureCanvas):
    """This is the canvas Widget. It allows for MPL plot embedding """
    
    def __init__(self, parent=None, width=9.85, height=5 , dpi=50):
        self.fig = Figure(figsize=(width, height), dpi=dpi)
        self.axes = self.fig.add_subplot(111)
        # We want the axes cleared every time plot() is called
        self.axes.hold(False)
        
        
        FigureCanvas.__init__(self, self.fig)
        self.setParent(parent)
        
        FigureCanvas.setSizePolicy(self,
                                   QtGui.QSizePolicy.Fixed,
                                   QtGui.QSizePolicy.Fixed)
        
        FigureCanvas.updateGeometry(self)


class MPLPlotter(QtGui.QWidget):
    """This is a widget that inherites from the plotWidget class that is used to update the plot with PP data"""
    def __init__(self, plotData, plotName, parent = None):
        QtGui.QWidget.__init__(self, parent)  
        
        
        self.plotData = plotData
        self.plotName = plotName
        self.canvas = PlotWidget()
        self.plotTitle = plotName + ' Postprocessor'
        self.getPlotColor()
        self.setPlotData(self.plotData, self.plotName)
        self.vbox = QtGui.QVBoxLayout()
        self.vbox.addWidget(self.canvas)
        self.setLayout(self.vbox)

        # set button context menu policy
        self.canvas.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
        self.connect(self.canvas, QtCore.SIGNAL('customContextMenuRequested(const QPoint&)'), self.on_context_menu)
        
        # create color menu
        self.colorMenu = QtGui.QMenu('Plot Color', self)
        royalBlueLine = QtGui.QAction('Blue',self)
        royalBlueLine.triggered.connect(self.changeRoyalBlue)
        orchidLine = QtGui.QAction('Magenta',self)
        orchidLine.triggered.connect(self.changeOrchid)
        tomatoLine = QtGui.QAction('Red',self)
        tomatoLine.triggered.connect(self.changeTomato)
        goldLine = QtGui.QAction('Yellow',self)
        goldLine.triggered.connect(self.changeGold)
        limeGreenLine = QtGui.QAction('Green',self)
        limeGreenLine.triggered.connect(self.changeLimeGreen)
        turquoiseLine = QtGui.QAction('Cyan',self)
        turquoiseLine.triggered.connect(self.changeTurquoise)
        blackLine = QtGui.QAction('Black',self)
        blackLine.triggered.connect(self.changeBlack)
        self.colorMenu.addAction(royalBlueLine)
        self.colorMenu.addAction(orchidLine)
        self.colorMenu.addAction(tomatoLine)
        self.colorMenu.addAction(goldLine)
        self.colorMenu.addAction(limeGreenLine)
        self.colorMenu.addAction(turquoiseLine)
        self.colorMenu.addAction(blackLine)



        # create context menu
        saveAction = QtGui.QAction('Save Plot', self)
        saveAction.triggered.connect(self.savePlot)
        closeAction = QtGui.QAction('Close Plot', self)
        closeAction.triggered.connect(self.closePlot)
        self.popMenu = QtGui.QMenu(self)
        self.popMenu.addAction(saveAction)
        self.popMenu.addSeparator()
        self.popMenu.addMenu(self.colorMenu)
        self.popMenu.addSeparator()
        self.popMenu.addAction(closeAction)

    
    def setPlotData(self, plotData, plotName):
        self.plotData = plotData
        self.plotName = plotName
        self.xData = self.plotData[0]
        self.yData = self.plotData[1]
        
        # MPL plots
        self.canvas.axes.plot(self.xData, self.yData, self.plotColor, linewidth = 2.5)
        self.canvas.axes.set_xlabel('time')
        self.canvas.axes.set_ylabel(self.plotName)
        self.canvas.axes.set_title(self.plotTitle)
        self.canvas.draw()

    def on_context_menu(self, point):
        # show context menu
            self.popMenu.exec_(self.canvas.mapToGlobal(point))

    def savePlot(self):
        path = unicode(QtGui.QFileDialog.getSaveFileName(self, 'Save file', self.plotTitle,"Images (*.pdf)" ))
        if path:
            self.canvas.print_figure(path, dpi = 100)
    def closePlot(self):
        self.close()
    
    def changeRoyalBlue(self):
        self.plotColor = "RoyalBlue"
        self.setPlotData(self.plotData,self.plotName)
    def changeOrchid(self):
        self.plotColor = "Magenta"
        self.setPlotData(self.plotData,self.plotName)
    def changeTomato(self):
        self.plotColor = "Tomato"
        self.setPlotData(self.plotData,self.plotName)
    def changeGold(self):
        self.plotColor = "Gold"
        self.setPlotData(self.plotData,self.plotName)
    def changeLimeGreen(self):
        self.plotColor = "LimeGreen"
        self.setPlotData(self.plotData,self.plotName)
    def changeTurquoise(self):
        self.plotColor = "DarkTurquoise"
        self.setPlotData(self.plotData,self.plotName)
    def changeBlack(self):
        self.plotColor = "Black"
        self.setPlotData(self.plotData,self.plotName)


    def getPlotColor(self):
        if (self.plotName[0] in ('a','A','f','F','k','K','p','P','u','U','z','Z')):
            self.plotColor = "LimeGreen"
        elif (self.plotName[0] in ('b','B','g','G','l','L','q','Q','v','V')):
            self.plotColor = "DarkTurquoise"
        elif (self.plotName[0] in ('c','C','h','H','m','M','r','R','w','W')):
            self.plotColor = "RoyalBlue"
        elif (self.plotName[0] in ('d','D','i','I','n','N','s','S','x','X')):
            self.plotColor = "Magenta"
        elif (self.plotName[0] in ('e','E','j','J','o','O','t','T','y','Y')):
            self.plotColor = "Tomato"
        else:
            self.plotColor = "Gold"
        