#!usr/bin/python

import sys, os, random
from PyQt4 import QtGui, QtCore
import numpy, csv
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure


class PlotWidget(FigureCanvas):
    """This is the canvas Widget. It allows for MPL plot embedding """
    
    def __init__(self, parent=None, width=7.75, height=4 , dpi=50):
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


class MPLPlotter(PlotWidget):
    """This is a widget that inherites from the plotWidget class that is used to update the plot with PP data"""
    def __init__(self, plotData, plotName):
        PlotWidget.__init__(self)
        
        self.setPlotData(plotData,plotName)
    
    def setPlotData(self, plotData, plotName):
        self.plotData = plotData
        self.plotName = plotName
        self.xData = self.plotData[0]
        self.yData = self.plotData[1]
        self.plotTitle = plotName + ' Postprocessor'
        
        
        # MPL plots
        self.axes.plot(self.xData, self.yData, 'r')
        self.axes.set_xlabel('time')
        self.axes.set_ylabel(self.plotName)
        self.axes.set_title(self.plotTitle)
        self.draw()