#!/usr/bin/python
from PyQt4 import QtCore, QtGui
from PyQt4.Qt import *

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s


class OptionsGUI(QtGui.QDialog):
  def __init__(self, main_data, single_item, incoming_data, win_parent=None):
    QtGui.QDialog.__init__(self, win_parent)
    self.main_data = main_data
    self.single_item = single_item
#    self.widget_list = []
    self.button_list =[]
#    self.layout_group = []
    self.layout_buttons = []
    self.incoming_data = incoming_data
    self.initUI()
    self.changed_cells = {}

  def initUI(self):
    # Init main window
    self.main_ui = QtGui.QWidget(self)
    self.main_ui.setObjectName(_fromUtf8("Dialog"))

    # Set layout
    self.layoutV = QtGui.QVBoxLayout(self)

    # build main window
    self.init_menu(self.layoutV)

    # combine it all together
    self.table_widget = QtGui.QTableWidget()
    self.layoutV.addWidget(self.table_widget)
    # self.main_ui.setLayout(self.layoutV)

    print self.incoming_data
    if self.incoming_data:
      self.drop_menu.setCurrentIndex(self.drop_menu.findText(self.incoming_data['type']))
      self.table_widget.cellChanged.connect(self.cellChanged)
      self.fillTableWithData(self.incoming_data)
      self.table_widget.cellChanged.disconnect(self.cellChanged)

    self.resize(700,500)

  ### Takes a dictionary containing name value pairs
  def fillTableWithData(self, the_data):
    for name,value in the_data.items():
      for i in xrange(0,self.total_rows):
        row_name = str(self.table_widget.item(i,0).text())

        if row_name == name:
          item = self.table_widget.item(i,1)
          item.setText(value)

  def tableToDict(self):
    the_data = {}
    for i in xrange(0,self.total_rows):
      param_name = str(self.table_widget.item(i,0).text())
      param_value = str(self.table_widget.item(i,1).text())
      if not param_name in self.original_table_data or not self.original_table_data[param_name] == param_value:
        the_data[param_name] = param_value
    return the_data
    

  def init_menu(self, layout):
    self.drop_menu = QtGui.QComboBox()
    for item in self.main_data:
      self.drop_menu.addItem(item['name'].split('/').pop())
#    self.drop_menu.activated[str].connect(self.item_clicked)
    self.drop_menu.currentIndexChanged[str].connect(self.item_clicked)
    layout.addWidget(self.drop_menu)

  def click_add(self):
    print 'add'
    self.table_data = self.tableToDict()
    self.accept()
    return

  def result(self):
    return self.table_data

  def click_cancel(self):
    print 'cancel'
    self.reject()

  def item_clicked(self, item):
    print "item_clicked"
    # Hide previous widgets (you can not delete them apparently)
    # TODO: theres gotta be a way to handle destroying widgets no
    # longer needed.
    for button in self.button_list:
      button.hide()

    try:
      self.table_widget.cellChanged.disconnect(self.cellChanged)
    except:
      pass

    # Build the Table
    the_table_data = []

    # Save off thie original data from the dump so we can compare later
    self.original_table_data = {}

    the_table_data.append({'name':'Name','default':'','description':'Name you want to give to this object','required':True})
    
    for new_text in self.main_data:
      if new_text['name'].split('/').pop() == item:
        the_table_data.append({'name':'type','default':new_text['name'].split('/').pop(),'description':'The object type','required':True})
        for param in new_text['parameters']:
          self.original_table_data[param['name']] = param['default']
          the_table_data.append(param)
        break

    self.total_rows = len(the_table_data)
    self.table_widget.setRowCount(self.total_rows)
    self.table_widget.setColumnCount(3)
    self.table_widget.setHorizontalHeaderItem(0, QtGui.QTableWidgetItem('Name'))
    self.table_widget.setHorizontalHeaderItem(1, QtGui.QTableWidgetItem('Value'))
    self.table_widget.setHorizontalHeaderItem(2, QtGui.QTableWidgetItem('Description'))
    self.table_widget.verticalHeader().setVisible(False)

    row = 0
    for param in the_table_data:
      # Populate table with data:
      name_item = QtGui.QTableWidgetItem(param['name'])

      value = ''
      
      if not param['required'] or param['name'] == 'type':
        value = param['default']
        
      value_item = QtGui.QTableWidgetItem(value)

      doc_item = QtGui.QTableWidgetItem(param['description'])
      
      name_item.setFlags(QtCore.Qt.ItemIsEnabled)
      doc_item.setFlags(QtCore.Qt.ItemIsEnabled)

      if param['name'] == 'type':
        value_item.setFlags(QtCore.Qt.NoItemFlags)

      self.table_widget.setItem(row, 0, name_item)
      self.table_widget.setItem(row, 1, value_item)
      self.table_widget.setItem(row, 2, doc_item)
      row += 1

    self.table_widget.resizeColumnsToContents()

    # Build the Add and Cancel buttons
    self.button_list.append(QtGui.QPushButton("Add"))
    self.button_list.append(QtGui.QPushButton("Cancel"))
    QtCore.QObject.connect(self.button_list[len(self.button_list) - 2], QtCore.SIGNAL("clicked()"), self.click_add)
    QtCore.QObject.connect(self.button_list[len(self.button_list) - 1], QtCore.SIGNAL("clicked()"), self.click_cancel)
    self.layout_buttons.append(QtGui.QHBoxLayout())
    for button in self.button_list:
      self.layout_buttons[len(self.layout_buttons) - 1].addWidget(button)
    self.layoutV.addLayout(self.layout_buttons[len(self.layout_buttons) - 1])
    
    self.table_widget.cellChanged.connect(self.cellChanged)
    
  def cellChanged(self, row, col):
    print "Changed!"
