[GlobalParams]
  density = 10800.0          # kg/m^3
  order = SECOND
  family = LAGRANGE
  disp_x = disp_x
[]

[Mesh]
  file = 1D-SPH_mesh.e
  displacements = 'disp_x'
  construct_side_list_from_node_list = true
[]

[Problem]
  coord_type = RSPHERICAL
[]

[Variables]
  [./disp_x]
  [../]
[]

[AuxVariables]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_r = disp_x
  [../]
[]

[AuxKernels]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
    execute_on = timestep
  [../]
[]

[BCs]
  [./exterior_pressure_x]
    type = Pressure
    variable = disp_x
    boundary = outer
    component = 0
    factor = 200
  [../]

  [./interior_pressure_x]
    type = Pressure
    variable = disp_x
    boundary = inner
    component = 0
    factor = 100
  [../]
[]

[Materials]
 [./fuel_disp]              
    type = Elastic
    block = 1
    disp_r = disp_x
    youngs_modulus = 1e10
    poissons_ratio = .345
    thermal_expansion = 0
  [../]

  [./fuel_den]
    type = Density
    block = 1
    disp_r = disp_x
  [../]
[]

[Debug]
    show_var_residual_norms = true
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'

  line_search = 'none'

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_rel_tol = 5e-6
  nl_abs_tol = 1e-10
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 50

   start_time = 0.0
   end_time = 1
   num_steps = 1000

  dtmax = 5e6
  dtmin = 1
  [./TimeStepper]
    type = AdaptiveDT
    dt = 1
    optimal_iterations = 6
    iteration_window = 0.4
    linear_iteration_ratio = 100
  [../]

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]

#  [./Quadrature]
#    order = THIRD
#  [../]
[]

[Postprocessors]
  [./dt]
    type = TimestepSize
  [../]
[]

[Output]
  linear_residuals = true
  file_base = 1D-SPH_out
   interval = 1
   output_initial = true
   exodus = true
   postprocessor_csv = false
   perf_log = true
[]
