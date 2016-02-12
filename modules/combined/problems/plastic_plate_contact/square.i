[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

#[Problem]
#  type = ReferenceResidualProblem
#  solution_variables = 'disp_x disp_y'
#  reference_residual_variables = 'saved_x saved_y'
#[]

[Mesh]
  file = plastic_plate.e
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
#  [./saved_x]
#  [../]
#  [./saved_y]
#  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./plastic_strain_mag]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
#    save_in_disp_x = saved_x
#    save_in_disp_y = saved_y
#    use_displaced_mesh = false
  [../]
[]

[AuxKernels]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]
  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
  [../]
  [./vonmises]
    type = MaterialTensorAux
    tensor = stress
    variable = vonmises
    quantity = VonMises
  [../]
  [./plastic_strain_mag]
    type = MaterialTensorAux
    tensor = plastic_strain
    variable = plastic_strain_mag
    quantity = PlasticStrainMag
  [../]
[]

[Functions]
  [./disp_top_y]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 0.1'
  [../]
[]

[Contact]
  [./releasing_surface]
    master = 5
    slave = 3
    disp_x = disp_x
    disp_y = disp_y
    penalty = 1e6
    model = glued
    formulation = kinematic
    system = Constraint
  [../]
[]

[BCs]
  [./top_y]
    type = FunctionPresetBC
    boundary = 7
    variable = disp_y
    function = disp_top_y
  [../]

  [./bottom_y]
    type = PresetBC
    boundary = 1
    variable = disp_y
    value = 0.0
  [../]

  [./right_x_lower]
    type = PresetBC
    boundary = 2
    variable = disp_x
    value = 0.0
  [../]

  [./right_x_upper]
    type = PresetBC
    boundary = 6
    variable = disp_x
    value = 0.0
  [../]
[]

[Materials]
  [./plastic_body1]
    type = LinearStrainHardening
#    formulation = linear
    block = 1
    poissons_ratio = 0.3
    youngs_modulus = 1e6
    yield_stress = 2e3
    hardening_constant = 1e4
    relative_tolerance = 1e-25
    absolute_tolerance = 1e-5
    disp_x = disp_x
    disp_y = disp_y
#    type = LinearIsotropicMaterial
#    block = 1
#    poissons_ratio = 0.3
#    youngs_modulus = 1e6
#    disp_x = disp_x
#    disp_y = disp_y
  [../]
  [./density_body1]
    type = Density
    block = 1
    density = 1.0
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
#  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
#  petsc_options_value = '201                hypre    boomeramg      8'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -ksp_gmres_restart'
  petsc_options_value = 'lu       superlu_dist                  101'


  line_search = 'cp'

#  [./Predictor]
#    type = SimplePredictor
#    scale = 1.0
#  [../]

# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-6

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-4
  nl_abs_tol = 1e-8

# time control
  start_time = 0.0
  dt = 0.05
  end_time = 1.00
  num_steps = 5000
[]


[Outputs]
  file_base = square_out
  output_initial = true
  exodus = true
#  vtk = true
#  gnuplot = true
  [./console]
    type = Console
    perf_log = true
    output_linear = true
  [../]
#  [./vtk]
#    type = GNUPlot
#    output_initial = true
#  [../]
[]
