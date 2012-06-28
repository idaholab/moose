#!/usr/bin/python
from PyQt4 import QtCore, QtGui
from PyQt4.Qt import *
from GenSyntax import *

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

class OptionsWidget(QtGui.QComboBox):
  def __init__(self, table_widget, row, options, is_vector_type):
    QtGui.QComboBox.__init__(self)
    self.table_widget = table_widget
    self.row = row
    self.is_vector_type = is_vector_type

    for option in options:
      self.addItem(str(option))

    self.setCurrentIndex(-1)  
    self.currentIndexChanged[str].connect(self.itemClicked)

  def itemClicked(self, item):
    table_value_item = self.table_widget.item(self.row,1)

    if not self.is_vector_type or table_value_item.text() == '':
      table_value_item.setText(item)
    else:
      table_value_item.setText(str(table_value_item.text()).strip("'") + ' ' + item)
  
    self.currentIndexChanged[str].disconnect(self.itemClicked)
    self.setCurrentIndex(-1)
    self.currentIndexChanged[str].connect(self.itemClicked)

class ParamTable:
  def __init__(self, main_data, action_syntax, single_options, incoming_data, main_layout, parent_class, already_has_parent_params, type_options):
    self.main_data = main_data
    self.action_syntax = action_syntax
    self.type_options = type_options

    if main_data and 'subblocks' in main_data:
      self.subblocks = main_data['subblocks']
    else:
      self.subblocks = None
      
    self.param_names = {}      
    self.original_table_data = {}
    self.incoming_data = incoming_data
    self.main_layout = main_layout
    self.parent_class = parent_class
    self.already_has_parent_params = already_has_parent_params
    self.initUI()

  def initUI(self):
#    self.layoutV = QtGui.QVBoxLayout(self.main_layout)
    self.layoutV = QtGui.QVBoxLayout()
    
    self.init_menu(self.layoutV)
    self.table_widget = QtGui.QTableWidget()
    self.table_widget.setColumnCount(4)
    self.table_widget.setHorizontalHeaderItem(0, QtGui.QTableWidgetItem('Name'))
    self.table_widget.setHorizontalHeaderItem(1, QtGui.QTableWidgetItem('Value'))
    self.table_widget.setHorizontalHeaderItem(2, QtGui.QTableWidgetItem('Options'))
    self.table_widget.setHorizontalHeaderItem(3, QtGui.QTableWidgetItem('Description'))
    self.table_widget.verticalHeader().setVisible(False)
    self.layoutV.addWidget(self.table_widget)

    self.drop_menu.setCurrentIndex(-1)
    found_index = self.drop_menu.findText('*')

    if found_index == -1:
      found_index = self.drop_menu.findText('ParentParams')
      
    self.drop_menu.setCurrentIndex(found_index)

    # print self.incoming_data
    if self.incoming_data:
      if 'type' in self.incoming_data and self.drop_menu.findText(self.incoming_data['type']) != -1:
        self.drop_menu.setCurrentIndex(self.drop_menu.findText(self.incoming_data['type']))
      else:
        if 'Name' in self.incoming_data:
          found_index = self.drop_menu.findText(self.incoming_data['Name'])
          if found_index != -1:
            self.drop_menu.setCurrentIndex(found_index)
              
      self.table_widget.cellChanged.connect(self.cellChanged)
      self.fillTableWithData(self.incoming_data, True)
      self.table_widget.cellChanged.disconnect(self.cellChanged)
    self.main_layout.addLayout(self.layoutV)

    apply_button = None
        # Build the Add and Cancel buttons
    if self.incoming_data and len(self.incoming_data):
      apply_button = QtGui.QPushButton("Apply")
    else:
      apply_button = QtGui.QPushButton("Add")

    new_row_button = QtGui.QPushButton("New Parameter")
    cancel_button = QtGui.QPushButton("Cancel")
    
    QtCore.QObject.connect(apply_button, QtCore.SIGNAL("clicked()"), self.click_add)
    QtCore.QObject.connect(new_row_button, QtCore.SIGNAL("clicked()"), self.click_new_row)
    QtCore.QObject.connect(cancel_button, QtCore.SIGNAL("clicked()"), self.click_cancel)
    
    button_layout = QtGui.QHBoxLayout()

    button_layout.addWidget(apply_button)
    button_layout.addWidget(new_row_button)
    button_layout.addWidget(cancel_button)
    
    self.layoutV.addLayout(button_layout)


  ### Takes a dictionary containing name value pairs
  def fillTableWithData(self, the_data, overwrite_type=False, old_params={}):
#     for name,value in the_data.items():
#       for i in xrange(0,self.table_widget.rowCount()):
#         row_name = str(self.table_widget.item(i,0).text())

#         if row_name == name:
#           item = self.table_widget.item(i,1)
#           item.setText(str(value))
    used_params = []
    # First, loop through and add all data that corresponds to YAML
    for i in xrange(0,self.table_widget.rowCount()):
      row_name = str(self.table_widget.item(i,0).text())

      if row_name in the_data and (overwrite_type == True or row_name != 'type'):
        if type(self.table_widget.cellWidget(i,1)) is QtGui.QComboBox:
          incoming_value = None
          if the_data[row_name] == '1' or the_data[row_name] == 'true':
            incoming_value = 'true'
          else:
            incoming_value = 'false'
            
          cb = self.table_widget.cellWidget(i,1)
          found_index = cb.findText(incoming_value)
          cb.setCurrentIndex(found_index)
        else:
          item = self.table_widget.item(i,1)
          item.setText(str(the_data[row_name]))
        used_params.append(row_name)
    # Now look to see if we have more data that wasn't in YAML and add additional rows for that
    for name,value in the_data.items():
      if name not in used_params and name != 'type' and name not in old_params:
        self.table_widget.insertRow(self.table_widget.rowCount())
        name_item = QtGui.QTableWidgetItem(name)
        value_item = QtGui.QTableWidgetItem(value)
        self.table_widget.setItem(self.table_widget.rowCount()-1,0,name_item)
        self.table_widget.setItem(self.table_widget.rowCount()-1,1,value_item)

  def tableToDict(self, only_not_in_original = False):
    the_data = {}
    for i in xrange(0,self.table_widget.rowCount()):
      param_name = str(self.table_widget.item(i,0).text())
      param_value = None
      if type(self.table_widget.cellWidget(i,1)) is QtGui.QComboBox:
        param_value = self.table_widget.cellWidget(i,1).currentText()
      else:
        param_value = str(self.table_widget.item(i,1).text())
        
      if param_value == '':
        continue

      if ' ' in param_value:
        param_value = "'"+param_value.strip("'")+"'"
        
      if not param_name in self.original_table_data or self.original_table_data[param_name] != param_value: #If we changed it - definitely include it
          the_data[param_name] = param_value
      else:
        if not only_not_in_original: # If we want stuff other than what we changed
          if param_name == 'parent_params' or param_name == 'type':  #Pass through type and parent_params even if we didn't change them
             the_data[param_name] = param_value
#          else:
#            if param_name in self.param_is_required and self.param_is_required[param_name]: #Pass through any 'required' parameters
#              the_data[param_name] = param_value
    return the_data
    

  def init_menu(self, layout):
    self.drop_menu = QtGui.QComboBox()
    self.has_type = False
    if self.subblocks:
      for item in self.subblocks:
        name = item['name'].split('/').pop()
        if name == '<type>':  #If this is the "type" node then put all of it's subblocks into the menu
          if self.already_has_parent_params:
            continue
          self.has_type = True
          if item['subblocks'] and len(item['subblocks']):
            for sb in item['subblocks']:
              sb_name = sb['name'].split('/').pop()
              self.drop_menu.addItem(sb_name)
        else:
          if not self.action_syntax.isPath(item['name']):
            self.drop_menu.addItem(name)
    

    if not self.already_has_parent_params and self.main_data and self.main_data['parameters'] and len(self.main_data['parameters']) and ('subblocks' not in self.main_data or not self.main_data['subblocks'] or not self.has_type):
      self.drop_menu.addItem('ParentParams')
#    self.drop_menu.activated[str].connect(self.item_clicked)
    self.drop_menu.currentIndexChanged[str].connect(self.item_clicked)
    layout.addWidget(self.drop_menu)

  def click_add(self):
#    self.table_widget.clearFocus()
    self.table_widget.setCurrentCell(0,0)
    self.table_data = self.tableToDict()
    self.parent_class.accept_params()
    return

  def click_new_row(self):
    self.table_widget.insertRow(self.table_widget.rowCount())

  def result(self):
    return self.table_data

  def click_cancel(self):
    self.parent_class.reject_params()

  def modifyCppType(self, param, new_text):
    if 'cpp_type' in param and param['cpp_type'] == 'bool':
      if param['default'] == '1':
        self.original_table_data[param['name']] = 'true'
      elif param['default'] == '0':
        self.original_table_data[param['name']] = 'false'
      else:
        self.original_table_data[param['name']] = ''
    else:
      self.original_table_data[param['name']] = param['default']

    if new_text != '' and param['name'] == 'type':
      param['default'] = new_text['name'].split('/').pop()

    if param['name'] == 'variable':
      param['cpp_type'] = 'VariableName'

    if param['name'] == 'block':
      param['cpp_type'] = 'std::vector<BlockName>'

    if param['name'] == 'boundary':
      param['cpp_type'] = 'std::vector<BoundaryName>'

    if param['name'] == 'function':
      param['cpp_type'] = 'FunctionName'

  def isVectorType(self, cpp_type):
    if 'vector' in cpp_type:
      return True

    if 'Vector' in cpp_type:
      return True

    return False
  
  def item_clicked(self, item):
    saved_data = {}
    saved_params = {}
    # Save off the current contents to try to restore it after swapping out the params
#    if self.original_table_data: #This will only have some length after the first time through
    saved_params = self.param_names #Save off the params from the previous YAML
    saved_data = self.tableToDict(True) # Pass true so we only save off stuff the user has entered

    self.table_widget.clearContents()

    # Build the Table
    the_table_data = []

    # Save off thie original data from the dump so we can compare later
    self.original_table_data = {}
    self.param_is_required = {}

    the_table_data.append({'name':'Name','default':'','description':'Name you want to give to this object','required':True})

    # Whether or not we've found the right data for this item
    found_it = False
    
    has_parent_params_set = False

    if self.subblocks:
      for new_text in self.subblocks:
        if new_text['name'].split('/').pop() == item:
          found_it = True
          #the_table_data.append({'name':'type','default':new_text['name'].split('/').pop(),'description':'The object type','required':True})
          for param in new_text['parameters']:
            self.modifyCppType(param, new_text)
            the_table_data.append(param)
            self.param_is_required[param['name']] = param['required']
          break #- can't break here because there might be more


      if not found_it and not self.already_has_parent_params: # If we still haven't found it... look under "item"
        for data in self.subblocks:
          name = data['name'].split('/').pop()
          if name == '<type>':
            if data['subblocks'] and len(data['subblocks']):
              for sb in data['subblocks']:
                sb_name = sb['name'].split('/').pop()
                if sb_name == item:
                  found_it = True
                  has_parent_params_set = True
                  for param in sb['parameters']:
                    self.modifyCppType(param, new_text)

                    the_table_data.append(param)
                    self.param_is_required[param['name']] = param['required']
          if found_it:
            break
    else:
      has_parent_params_set = True  #If there are no subblocks then these options are definitely going into the parent

    if item == 'ParentParams': # If they explicitly selected ParentParams then let's put them there
      has_parent_params_set = True

    if has_parent_params_set: # Need to add in the parent's params
      the_table_data.append({'name':'parent_params','default':'true','description':'These options will go into the parent','required':False})
      for param in self.main_data['parameters']:
        if param['name'] == 'type':
          continue

        self.modifyCppType(param, '')
          
        the_table_data.append(param)
        self.param_is_required[param['name']] = param['required']

    total_rows = len(the_table_data)
    self.table_widget.setRowCount(total_rows)

    row = 0

    self.param_names = []
    name_to_param = {}

    for param in the_table_data:
      self.param_names.append(param['name'])
      name_to_param[param['name']] = param

    for param_name in sorted(self.param_names):
      param = name_to_param[param_name]
      # Populate table with data:
      name_item = QtGui.QTableWidgetItem(param['name'])

      value = ''
      if not param['required'] or param['name'] == 'type':
        value = param['default']

      if param['required']:
        color = QtGui.QColor()
#        color.setNamedColor("red")
        color.setRgb(255,204,153)
        name_item.setBackgroundColor(color)

      if has_parent_params_set and param['name'] == 'Name':
        value = 'ParentParams'

      value_item = None

      if 'cpp_type' in param and param['cpp_type'] == 'bool':
        value_item = QtGui.QComboBox()
        value_item.addItem('')
        value_item.addItem('true')
        value_item.addItem('false')

        if value == '1':
          value_item.setCurrentIndex(1)
        elif value == '0':
          value_item.setCurrentIndex(2)
        else:
          value_item.setCurrentIndex(0)
          
        self.table_widget.setCellWidget(row, 1, value_item)
      else:
        value_item = QtGui.QTableWidgetItem(value)
        self.table_widget.setItem(row, 1, value_item)

      if 'cpp_type' in param and param['cpp_type'] in self.type_options:
        options_item = OptionsWidget(self.table_widget,row,self.type_options[param['cpp_type']], self.isVectorType(param['cpp_type']))
        self.table_widget.setCellWidget(row, 2, options_item)

      if 'cpp_type' in param and param['cpp_type'] == 'MooseEnum':
        options_item = OptionsWidget(self.table_widget,row,param['options'].split(' '), self.isVectorType(param['cpp_type']))
        self.table_widget.setCellWidget(row, 2, options_item)
      
      doc_item = QtGui.QTableWidgetItem(param['description'])
      
      name_item.setFlags(QtCore.Qt.ItemIsEnabled)
      doc_item.setFlags(QtCore.Qt.ItemIsEnabled)

      if (param['name'] == 'type' and str(item) != '*') or param['name'] == 'parent_params' or (has_parent_params_set and param['name'] == 'Name'):
        value_item.setFlags(QtCore.Qt.NoItemFlags)

      self.table_widget.setItem(row, 0, name_item)
      self.table_widget.setItem(row, 3, doc_item)
      row += 1

    self.table_widget.resizeColumnsToContents()

    # Try to restore saved data
    if len(saved_data):
      self.fillTableWithData(saved_data, old_params=saved_params)
    
    self.table_widget.cellChanged.connect(self.cellChanged)
    
  def cellChanged(self, row, col):
    pass
