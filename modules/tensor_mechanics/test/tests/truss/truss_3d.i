[Mesh]
  type = FileMesh
  file = truss_3d.e
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
 [./axial_stress]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e_over_l]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./area]
    order = CONSTANT
    family = MONOMIAL
#    initial_condition = 1.0
  [../]
  [./react_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./react_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./react_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./x2]
    type = PiecewiseLinear
    x = '0  1 2 3'
    y = '0 .5 1 1'
  [../]
  [./y2]
    type = PiecewiseLinear
    x = '0 1  2 3'
    y = '0 0 .5 1'
  [../]
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    preset = false
    boundary = 1
    value = 0.0
  [../]
  [./fixx2]
    type = FunctionDirichletBC
    variable = disp_x
    preset = false
    boundary = 2
    function = x2
  [../]
  [./fixx3]
    type = DirichletBC
    variable = disp_x
    preset = false
    boundary = 3
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    preset = false
    boundary = 1
    value = 0
  [../]
  [./fixy2]
    type = FunctionDirichletBC
    variable = disp_y
    preset = false
    boundary = 2
    function = y2
  [../]
  [./fixy3]
    type = DirichletBC
    variable = disp_y
    preset = false
    boundary = 3
    value = 0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    preset = false
    boundary = 1
    value = 0
  [../]
  [./fixz2]
    type = DirichletBC
    variable = disp_z
    preset = false
    boundary = 2
    value = 0
  [../]
  [./fixz3]
    type = DirichletBC
    variable = disp_z
    preset = false
    boundary = 3
    value = 0
  [../]
[]

[AuxKernels]
  [./axial_stress]
    type = MaterialRealAux
    block = '1 2'
    property = axial_stress
    variable = axial_stress
  [../]
  [./e_over_l]
    type = MaterialRealAux
    block = '1 2'
    property = e_over_l
    variable = e_over_l
  [../]
  [./area]
    type = ConstantAux
    block = '1 2'
    variable = area
    value = 1.0
    execute_on = 'initial timestep_begin'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'jacobi   101'
  line_search = 'none'

  nl_max_its = 15
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-10

  dt = 1
  num_steps = 3
  end_time = 3
[]

[Kernels]
  [./solid_x]
    type = StressDivergenceTensorsTruss
    block = '1 2'
    displacements = 'disp_x disp_y disp_z'
    component = 0
    variable = disp_x
    area = area
    save_in = react_x
  [../]
  [./solid_y]
    type = StressDivergenceTensorsTruss
    block = '1 2'
    displacements = 'disp_x disp_y disp_z'
    component = 1
    variable = disp_y
    area = area
    save_in = react_y
  [../]
  [./solid_z]
    type = StressDivergenceTensorsTruss
    block = '1 2'
    displacements = 'disp_x disp_y disp_z'
    component = 2
    variable = disp_z
    area = area
    save_in = react_z
  [../]
[]

[Materials]
  [./linelast]
    type = LinearElasticTruss
    block = '1 2'
    youngs_modulus = 1e6
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[Outputs]
  exodus = true
[]
