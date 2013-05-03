from PyQt4 import QtCore, QtGui
import MeshInfoFactory
from MeshRenderWidget import *

from InputFileWidget import *
from ExecuteWidget import *
from PostprocessorWidget import *
from ExodusResultRenderWidget import *

class PeacockApplication(object):
  def __init__(self, main_window):
    self.main_window = main_window

  ''' Should create and return a map of "Tab Name" to the associated Tab object in the order you want them to show up in Peacock.
      For the main tabs (input_file_widget, execute_widget, postprocessor_widget and visualize_widget) it should also set those
      member variables on the main_ui object that is passed in.'''
  def tabs(self, main_ui):
    tabs = []

    main_ui.input_file_widget = InputFileWidget(main_ui.app_path, main_ui.options, main_ui, main_ui.qt_app, main_ui.application)
    main_ui.execute_widget = ExecuteWidget(main_ui.app_path, main_ui.input_file_widget, main_ui.qt_app)
    main_ui.postprocessor_widget = PostprocessorWidget(main_ui.input_file_widget, main_ui.execute_widget)
    main_ui.visualize_widget = ExodusResultRenderWidget(main_ui.input_file_widget, main_ui.execute_widget, main_ui.qt_app, main_ui.application)

    tabs.append(main_ui.input_file_widget)
    tabs.append(main_ui.execute_widget)
    tabs.append(main_ui.postprocessor_widget)
    tabs.append(main_ui.visualize_widget)

    return tabs


  ''' This function is responsible for filling in the valid options for each "cpp_type" for parameters.
      The return value must be a dictionary... where the key is the cpp_type and the value is a set()
      of options.  This default implementation works well for most MOOSE based applications. '''
  def typeOptions(self):
    input_file_widget = self.main_window.input_file_widget
    tree_widget = input_file_widget.tree_widget
    
    type_options = {}

    # Variables
    variable_names = tree_widget.getChildNamesOfPath('Variables')
    type_options['std::vector<NonlinearVariableName, std::allocator<NonlinearVariableName> >'] = set()
    type_options['std::vector<NonlinearVariableName>'] = set()
    type_options['NonlinearVariableName'] = set()

    type_options['std::vector<VariableName, std::allocator<VariableName> >'] = set()
    type_options['std::vector<VariableName>'] = set()
    type_options['VariableName'] = set()

    if len(variable_names):
      type_options['std::vector<NonlinearVariableName, std::allocator<NonlinearVariableName> >'] |= set(variable_names)
      type_options['std::vector<NonlinearVariableName>'] |= set(variable_names)
      type_options['NonlinearVariableName'] |= set(variable_names)

      type_options['std::vector<VariableName, std::allocator<VariableName> >'] |= set(variable_names)
      type_options['std::vector<VariableName>'] |= set(variable_names)
      type_options['VariableName'] |= set(variable_names)

    # Aux Vars
    aux_variable_names = tree_widget.getChildNamesOfPath('AuxVariables')
    if len(aux_variable_names):
      type_options['std::vector<AuxVariableName, std::allocator<AuxVariableName> >'] = set(aux_variable_names)
      type_options['std::vector<AuxVariableName>'] = set(aux_variable_names)
      type_options['AuxVariableName'] = set(aux_variable_names)
        
      type_options['std::vector<VariableName, std::allocator<VariableName> >'] |= set(aux_variable_names)
      type_options['std::vector<VariableName>'] |= set(aux_variable_names)
      type_options['VariableName'] |= set(aux_variable_names)

    # Functions
    function_names = tree_widget.getChildNamesOfPath('Functions')
    if len(function_names):
      type_options['std::vector<FunctionName, std::allocator<FunctionName> >'] = set(function_names)
      type_options['std::vector<FunctionName>'] = set(function_names)
      type_options['FunctionName'] = set(function_names)

    # UserObjects
    user_object_names = tree_widget.getChildNamesOfPath('UserObjects')
    if len(user_object_names):
      type_options['std::vector<UserObjectName, std::allocator<UserObjectName> >'] = set(user_object_names)
      type_options['std::vector<UserObjectName>'] = set(user_object_names)
      type_options['UserObjectName'] = set(user_object_names)

    # Markers
    marker_names = tree_widget.getChildNamesOfPath('Adaptivity/Markers')
    if len(marker_names):
      type_options['std::vector<MarkerName, std::allocator<MarkerName> >'] = set(marker_names)
      type_options['std::vector<MarkerName>'] = set(marker_names)
      type_options['MarkerName'] = set(marker_names)

    # Indicators
    indicator_names = tree_widget.getChildNamesOfPath('Adaptivity/Indicators')
    if len(indicator_names):
      type_options['std::vector<IndicatorName, std::allocator<IndicatorName> >'] = set(indicator_names)
      type_options['std::vector<IndicatorName>'] = set(indicator_names)
      type_options['IndicatorName'] = set(indicator_names)

    # MultiApps
    multi_app_names = tree_widget.getChildNamesOfPath('MultiApps')
    if len(multi_app_names):
      type_options['std::vector<MultiAppName, std::allocator<MultiAppName> >'] = set(multi_app_names)
      type_options['std::vector<MultiAppName>'] = set(multi_app_names)
      type_options['MultiAppName'] = set(multi_app_names)

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

  ''' This function allows you to apply VTK filters to the result before it is rendered in the visualize widget.
      The incoming object is vtkPolyData... you will most likely want to create a VTK filter and then attach the output of the result_vtk_object
      to the input of your filter.
      The return value of this function MUST be a VTK object that provides vtkPolyData on its output port '''
  def filterResult(self, result_vtk_object):
    return result_vtk_object

  def addExodusResultActors(self, appRenderer):
    pass


  def addRelapSave(self, layout):       
    pass

  def addNumberHistory(self, command_layout):
    pass
