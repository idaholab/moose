[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  file = 2squares.e
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./strain_zz]
  [../]
  [./temp]
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./aux_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Postprocessors]
  [./react_z]
    type = MaterialTensorIntegralSM
    tensor = stress
    index = 2
  [../]
  [./min_strain_zz]
    type = NodalExtremeValue
    variable = strain_zz
    value_type = min
  [../]
  [./max_strain_zz]
    type = NodalExtremeValue
    variable = strain_zz
    value_type = max
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    temp = temp
  [../]
[]

[Kernels]
  [./solid_z]
    type = OutOfPlaneStress
    variable = strain_zz
    disp_x = disp_x
    disp_y = disp_y
    temp = temp
  [../]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
[]

[Constraints]
  [./szz]
    type = EqualValueBoundaryConstraint
    variable = strain_zz
    master = 2
    slave = 10
    penalty = 1e12
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
  [../]
  [./stress_xy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xy
    index = 3
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
  [../]

  [./strain_xx]
    type = MaterialTensorAux
    tensor = total_strain
    variable = strain_xx
    index = 0
  [../]
  [./strain_xy]
    type = MaterialTensorAux
    tensor = total_strain
    variable = strain_xy
    index = 3
  [../]
  [./strain_yy]
    type = MaterialTensorAux
    tensor = total_strain
    variable = strain_yy
    index = 1
  [../]
  [./strain_zz]
    type = MaterialTensorAux
    tensor = total_strain
    variable = aux_strain_zz
    index = 2
  [../]
[]

[Functions]
  [./tempramp]
    type = ParsedFunction
    value = 't'
  [../]
[]

[BCs]
  [./x]
    type = DirichletBC
    boundary = '4 6'
    variable = disp_x
    value = 0.0
  [../]
  [./y]
    type = DirichletBC
    boundary = '1 5' #'4 6'
    variable = disp_y
    value = 0.0
  [../]
  [./t]
    type = DirichletBC
    boundary = '4'
    variable = temp
    value = 0.0
  [../]
  [./tramp]
    type = FunctionPresetBC
    variable = temp
    boundary = '6'
    function = tempramp
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
#    full = true
    off_diag_row =    'disp_x disp_y'
    off_diag_column = 'disp_y disp_x'
  [../]
[]

[Contact]
  [./mech]
    master = 8
    slave = 2
    disp_x = disp_x
    disp_y = disp_y
    penalty = 1e+10
    normalize_penalty = true
    system = Constraint
    tangential_tolerance = .1
    normal_smoothing_distance = .1
    model = frictionless
    formulation = kinematic
  [../]
[]

[ThermalContact]
  [./thermal]
    type = GapHeatTransfer
    master = 8
    slave = 2
    variable = temp
    tangential_tolerance = .1
    normal_smoothing_distance = .1
    gap_conductivity = 0.01
    min_gap = 0.001
#    quadrature = true
  [../]
[]

[Materials]
  [./linelast]
    type = Elastic
    block = '1 2'
    disp_x = disp_x
    disp_y = disp_y
    poissons_ratio = 0.3
    youngs_modulus = 1e6
    thermal_expansion = 0.02
    stress_free_temperature = 0.0
    temp = temp
    formulation = PlaneStrain
    strain_zz = strain_zz
  [../]
  [./heatcond]
    type = HeatConductionMaterial
    block = '1 2'
    thermal_conductivity = 3.0
    specific_heat = 300.0
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  line_search = none

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'

# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-4

# controls for nonlinear iterations
  nl_max_its = 20
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-4

# time control
  start_time = 0.0
  dt = 0.2
  dtmin = 0.2
  end_time = 2.0
  num_steps = 5000
[]

[Outputs]
  exodus = true
[]
