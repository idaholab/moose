[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 4
[]

[XFEM]
  geometric_cut_userobjects = 'line_seg_cut_uo'
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [./line_seg_cut_uo]
    type = LineSegmentCutUserObject
    cut_data = '0.5 1.0 0.5 0.0'
    time_start_cut = 0.0
    time_end_cut = 2
  [../]
[]

[UserObjects]
  [./manager]
    type = XFEMElemPairMaterialManager
    material_names = 'material1 material2'
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Functions]
  [./u_left]
    type = PiecewiseLinear
    x = '0   2'
    y = '0  0.1'
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[Constraints]
  [./xfem_constraint]
    type = XFEMSingleVariableConstraintStatefulTest
    variable = u
    jump = 0
    jump_flux = 0
    manager = manager
    base_name = A
  [../]
[]

[BCs]
# Define boundary conditions
  [./left_u]
    type = FunctionPresetBC
    variable = u
    boundary = 3
    function = u_left
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
[]

[Materials]
  [./material1]
    type = StatefulMaterialJump
    base_name = A
    compute = false
    u = u
  [../]
  [./material2]
    type = StatefulMaterialJump
    base_name = B
    compute = false
    u = u
  [../]
[]

[Kernels]
  [./dt]
    type = TimeDerivative
    variable = u
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 2
[]

[Outputs]
  execute_on = timestep_end
  exodus = true
[]
