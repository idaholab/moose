from PyQt4 import QtCore, QtGui
import MeshInfoFactory
from MeshRenderWidget import *

class PeacockApplication(object):
  def __init__(self, main_window):
    self.main_window = main_window

  ''' This function is responsible for filling in the valid options for each "cpp_type" for parameters.
      The return value must be a dictionary... where the key is the cpp_type and the value is a set()
      of options.  This default implementation works well for most MOOSE based applications. '''
  def typeOptions(self):
    input_file_widget = self.main_window.input_file_widget
    tree_widget = input_file_widget.tree_widget
    
    type_options = {}

    # Variables
    variables_items = input_file_widget.tree_widget.findItems("Variables", QtCore.Qt.MatchExactly)
    if variables_items:
      variables_item = variables_items[0]
      variable_names = tree_widget.getChildNames(variables_item)
      if len(variable_names):
        type_options['std::vector<NonlinearVariableName, std::allocator<NonlinearVariableName> >'] = set(variable_names)
        type_options['std::vector<NonlinearVariableName>'] = set(variable_names)
        type_options['NonlinearVariableName'] = set(variable_names)

        type_options['std::vector<VariableName, std::allocator<VariableName> >'] = set(variable_names)
        type_options['std::vector<VariableName>'] = set(variable_names)
        type_options['VariableName'] = set(variable_names)

    # Aux Vars
    aux_variables_items = input_file_widget.tree_widget.findItems("AuxVariables", QtCore.Qt.MatchExactly)
    if aux_variables_items:
      aux_variables_item = aux_variables_items[0]
      aux_variable_names = tree_widget.getChildNames(aux_variables_item)
      if len(aux_variable_names):
        type_options['std::vector<AuxVariableName, std::allocator<AuxVariableName> >'] = set(aux_variable_names)
        type_options['std::vector<AuxVariableName>'] = set(aux_variable_names)
        type_options['AuxVariableName'] = set(aux_variable_names)
        
        type_options['std::vector<VariableName, std::allocator<VariableName> >'] |= set(aux_variable_names)
        type_options['std::vector<VariableName>'] |= set(aux_variable_names)
        type_options['VariableName'] |= set(aux_variable_names)

    # Functions
    functions_items = input_file_widget.tree_widget.findItems("Functions", QtCore.Qt.MatchExactly)
    if functions_items:
      functions_item = functions_items[0]
      function_names = tree_widget.getChildNames(functions_item)
      if len(function_names):
        type_options['std::vector<FunctionName, std::allocator<FunctionName> >'] = set(function_names)
        type_options['std::vector<FunctionName>'] = set(function_names)
        type_options['FunctionName'] = set(function_names)

    # Userobjects
    userobjects_items = input_file_widget.tree_widget.findItems("UserObjects", QtCore.Qt.MatchExactly)
    if userobjects_items:
      userobjects_item = userobjects_items[0]
      userobject_names = tree_widget.getChildNames(userobjects_item)
      if len(userobject_names):
        type_options['std::vector<UserObjectName, std::allocator<UserObjectName> >'] = set(userobject_names)
        type_options['std::vector<UserObjectName>'] = set(userobject_names)
        type_options['UserObjectName'] = set(userobject_names)

    # Mesh stuff
    mesh_data = tree_widget.getMeshItemData()
    if mesh_data:
      mesh_info = MeshInfoFactory.getMeshInfo(mesh_data)

      if mesh_info:
        type_options['std::vector<BlockName>'] = mesh_info.blockNames()
        type_options['BlockName'] = mesh_info.blockNames()
        
        type_options['std::vector<BoundaryName, std::allocator<BoundaryName> >'] = mesh_info.sidesetNames()
        type_options['std::vector<BoundaryName>'] = mesh_info.sidesetNames()
        type_options['BoundaryName'] = mesh_info.sidesetNames()
        
        type_options['std::vector<BoundaryName, std::allocator<BoundaryName> >'].update(mesh_info.nodesetNames())
        type_options['std::vector<BoundaryName>'].update(mesh_info.nodesetNames())
        type_options['BoundaryName'].update(mesh_info.nodesetNames())

        type_options['std::vector<SubdomainName, std::allocator<SubdomainName> >'] = mesh_info.blockNames()
        type_options['std::vector<SubdomainName>'] = mesh_info.blockNames()
        type_options['SubdomainName'] = mesh_info.blockNames()
      
    return type_options

  ''' This is the graphical view of the mesh that is shown next to the input file tree view.
      This function should return a QWidget derived class.'''
  def meshRenderWidget(self, input_file_widget):
    return MeshRenderWidget(input_file_widget.tree_widget)

  ''' Whether or not to show the meshrenderwidget by default.
      For normal MOOSE based applications this is False and the meshrenderwidget is only shown
      after the Mesh block has been edited.  But, some applications don't have Mesh blocks...
      The return value is a boolean. '''
  def showMeshRenderWidgetByDefault(self):
    return False
