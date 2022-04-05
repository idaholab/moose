# Test for multi app vector postprocessor to aux variable transfer

# Master App contains 2 beams, one starting at (1.5, 2.0, 2.0) and
# another starting at (2.5, 0.0, 3.0). Both beams extend for
# 0.150080 m along the y direction.

# Each subApp contains a 2D model of width 0.5 m and height 0.150080 m.
# A time varying temperature profile is assigned to each 2D model and
# the resulting yy strain along the right boundary (x = 0.5) is
# transferred to the beam model using the multi app transfer. The subApps
# are positioned in the [MultiApp] block such that the origin of the beams
# coincides with the origin of the subApp.

# For each master beam node/element, the MultiAppUserObjectTransfer finds
# the subApp where this node belongs, projects this node to the right
# boundary of the subApp and assigns the value corresponding to the
# projected point.

# Result: The y displacement of the beam should equal the y
# displacement along the right boundary of the 2D model.

[Mesh]
  type = FileMesh
  file = 2_beams_new.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./rot_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = 1
    value = 0.0
  [../]
  [./fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = 1
    value = 0.0
  [../]
  [./fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = 1
    value = 0.0
  [../]
  [./fixr3]
    type = DirichletBC
    variable = rot_z
    boundary = 1
    value = 0.0
  [../]
[]

[Kernels]
  [./solid_disp_x]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 0
    variable = disp_x
  [../]
  [./solid_disp_y]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 1
    variable = disp_y
  [../]
  [./solid_disp_z]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 2
    variable = disp_z
  [../]
  [./solid_rot_x]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 3
    variable = rot_x
  [../]
  [./solid_rot_y]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 4
    variable = rot_y
  [../]
  [./solid_rot_z]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 5
    variable = rot_z
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeElasticityBeam
    youngs_modulus = 1e9
    poissons_ratio = 0.3
    shear_coefficient = 1.0
    block = 1
  [../]
  [./strain]
    type = ComputeIncrementalBeamStrain
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    area = 0.5
    Ay = 0.0
    Az = 0.0
    Iy = 0.01
    Iz = 0.01
    y_orientation = '0.0 0.0 1.0'
    eigenstrain_names = 'thermal'
  [../]
  [./stress]
    type = ComputeBeamResultants
    block = 1
  [../]
  [./thermal]
    type = ComputeEigenstrainBeamFromVariable
    displacement_eigenstrain_variables = 'zero1 to_var zero2'
    eigenstrain_name = thermal
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  line_search = 'none'

  l_max_its = 50
  nl_max_its = 50
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
  l_tol = 1e-9

  start_time = 0.0
  end_time = 0.075
  dt = 0.0125
  dtmin = 0.0001
[]

[AuxVariables]
  [./to_var]
  [../]
  [./zero1]
  [../]
  [./zero2]
  [../]
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    app_type = CombinedApp
    positions = '1.5 2.0 2.0 2.5 0.0 3.0'
    input_files = 'subapp1_uo_transfer.i subapp2_uo_transfer.i'
  [../]
[]

[Transfers]
  [./fromsub]
    type = MultiAppUserObjectTransfer
    user_object = axial_str
    from_multi_app = sub
    variable = to_var
    all_master_nodes_contained_in_sub_app = true
  [../]
[]

[Postprocessors]
  [./pos1]
    type = PointValue
    variable = disp_y
    point = '1.5 2.150080 2.0'
  [../]
  [./pos2]
    type = PointValue
    variable = disp_y
    point = '2.5 0.150080 3.0'
  [../]
[]

[Outputs]
  exodus = true
[]
