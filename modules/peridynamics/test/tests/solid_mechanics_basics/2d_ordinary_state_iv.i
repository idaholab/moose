# Test for ordinary state-based peridynamic formulation
# for irregular grid from file mesh with varying bond constants
# partial Jacobian
# Jacobian from bond-based formulation is used for preconditioning

# Square plate with Dirichlet boundary conditions applied
# at the left, top and bottom edges

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = FileMeshPD
  file = square.e
  horizon_number = 3
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[BCs]
  [./left_dx]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./top_dy]
    type = DirichletBC
    variable = disp_y
    boundary = 4
    value = 0.0
  [../]
  [./bottom_dy]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 2
    function = '-0.001 * t'
  [../]
[]

[Modules]
  [./Peridynamics]
    [./Mechanics]
      formulation = OrdinaryState
    [../]
  [../]
[]

[Materials]
  [./linelast]
    type = SmallStrainVariableHorizonOSPD
    youngs_modulus = 2e5
    poissons_ratio = 0.0
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  line_search = none
  start_time = 0
  end_time = 1
[]

[Outputs]
  file_base = 2d_ordinary_state_iv
  exodus = true
[]
