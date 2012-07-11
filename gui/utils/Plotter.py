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
        
        
            
        self.canvas = PlotWidget()
        self.setPlotData(plotData,plotName)
        self.vbox = QtGui.QVBoxLayout()
        self.vbox.addWidget(self.canvas)
        self.setLayout(self.vbox)

        # set button context menu policy
        self.canvas.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
        self.connect(self.canvas, QtCore.SIGNAL('customContextMenuRequested(const QPoint&)'), self.on_context_menu)

        # create context menu
        saveAction = QtGui.QAction('Save', self)
        saveAction.triggered.connect(self.savePlot)
        #zoomAction = QtGui.QAction('Zoom', self)
        #panAction = QtGui.QAction('Pan', self)
        closeAction = QtGui.QAction('Close', self)
        closeAction.triggered.connect(self.closePlot)
        self.popMenu = QtGui.QMenu(self)
        self.popMenu.addAction(saveAction)
        #self.popMenu.addSeparator()
        #self.popMenu.addAction(zoomAction)
        #self.popMenu.addAction(panAction)
        self.popMenu.addSeparator()
        self.popMenu.addAction(closeAction)

    
    def setPlotData(self, plotData, plotName):
        self.plotData = plotData
        self.plotName = plotName
        self.xData = self.plotData[0]
        self.yData = self.plotData[1]
        self.plotTitle = plotName + ' Postprocessor'
        
        
        # MPL plots
        self.canvas.axes.plot(self.xData, self.yData, 'r')
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
        