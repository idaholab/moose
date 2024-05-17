# checks that the stress averages computed by CrackFrontStress are correct

# Left Crack Tip: moves in y with stress_11=y and crackFrontStress is equal to y_pos+box_length/2
# At boundary when the crackFrontStress box intersects the boundary, crackFrontStress is properly scaled

# Right Crack Tip: moves at 45deg with stress_10&_01=y.  crackFrontStress is equal to y_pos+box_length/sqrt(8)

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    # nx = 90
    # ny = 30
    nx = 400
    ny = 140
    xmin = -0.4
    xmax = 0.5
    ymin = 0.4
    ymax = 0.65
    elem_type = QUAD4
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[XFEM]
  geometric_cut_userobjects = 'cut_mesh2'
  qrule = volfrac
  output_cut_plane = true
[]

[Functions]
  [growth_func_x]
    type = ParsedFunction
    expression = 'if(x<0, 0, 1)'
  []
  [growth_func_y]
    type = ParsedFunction
    expression = 1
  []
  [growth_func_v]
    type = ParsedFunction
    expression = '0.05'
  []
[]

[UserObjects]
  [cut_mesh2]
    type = MeshCut2DFunctionUserObject
    mesh_file = make_edge_crack_in.e
    growth_direction_x = growth_func_x
    growth_direction_y = growth_func_y
    growth_rate = growth_func_v
    crack_front_definition = crack_tip
  []
  [crack_tip]
    type = CrackFrontDefinition
    crack_direction_method = CurvedCrackFront
    2d = true
    crack_front_points_provider = cut_mesh2
    number_points_from_provider = 2
  []
[]

[AuxVariables]
  [disp_x]
    family = LAGRANGE
    order = FIRST
  []
  [disp_y]
    family = LAGRANGE
    order = FIRST
  []
[]

[Functions]
  [fcn_00]
    type = ParsedFunction
    expression = 'if(x<0,y,0)'
  []
  [fcn_10]
    type = ParsedFunction
    expression = 'if(x<0,0,y)'
  []
  [fcn_20]
    type = ParsedFunction
    expression = '0'
  []
  [fcn_11]
    type = ParsedFunction
    expression = '0'
  []
  [fcn_21]
    type = ParsedFunction
    expression = '0'
  []
  [fcn_22]
    type = ParsedFunction
    expression = '0'
  []
[]

[Materials]
  [tensor]
    type = GenericFunctionRankTwoTensor
    tensor_name = generic_stress
    # tensor values are column major-ordered
    tensor_functions = 'fcn_00 fcn_10 fcn_20  fcn_10 fcn_11 fcn_21  fcn_20 fcn_21 fcn_22'
    outputs = all
  []
[]

[VectorPostprocessors]
  [CrackFrontNormalStress]
    type = CrackFrontNormalStress
    base_name = generic
    crack_front_definition = crack_tip
    box_length = 0.1
    box_height = 0.05
    box_width = 0.05
  []
[]

[Executioner]
  type = Transient
  # time control
  start_time = 0.0
  dt = 1.0
  end_time = 4
[]

[Outputs]
  exodus = true
  csv = true
  # execute_on = TIMESTEP_END
  [xfemcutter]
    type = XFEMCutMeshOutput
    xfem_cutter_uo = cut_mesh2
  []
[]
