[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = false
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 2
    nz = 1
    xmin = 9.4615
    xmax = 92.0115
    ymin = 3.175
    ymax = 22.225
    zmin = 0.489
    zmax = 0.755
  []
[]

[Variables]
  [T]
    scaling = 10
  []
  [lam_T]
    solver_sys = adjoint
    scaling = 1e3
  []
[]

[Kernels]
  [heat_conduction]
    type = ADMatDiffusion
    variable = T
    diffusivity = thermal_conductivity
  []
  [heat_source]
    type = ADBodyForce
    function = src_fuel_function
    variable = T
  []
[]

[BCs]
  [dir_BC_front]
    type = NeumannBC
    variable = T
    boundary = front
    value = 2
  []
  [dir_BC_back]
    type = DirichletBC
    variable = T
    boundary = back
    value = 300
  []
[]

[Materials]
  # fuel properties
  [fuel_thermal]
    type = ADGenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 17.6e3
  []
[]

[Executioner]
  type = SteadyAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu mumps'

  line_search = 'none'

  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-30
  nl_max_its = 10

  l_max_its = 10
[]

##---------Forward Optimization stuff------------------#
[Reporters]
  [measure_data]
    type = ConstantReporter
    real_vector_names = 'x y z u weight'
    real_vector_values = '0.2 0.2 0.0; 0.3 0.8 0.0; 0 0 0; 5 5 5; 1 1 1'
  []
  [params_fuel]
    type = ConstantReporter
    real_vector_names = 'source'
    real_vector_values = '5e7' # Dummy
  []
[]

[Functions]
  [src_fuel_function]
    type = ParsedOptimizationFunction
    expression = q
    param_symbol_names = 'q'
    param_vector_name = 'params_fuel/source'
  []
[]
##---------Adjoint Optimization stuff------------------#
[DiracKernels]
  [adjointLoad_T]
    type = ReporterPointSource
    variable = lam_T
    x_coord_name = measure_data/x
    y_coord_name = measure_data/y
    z_coord_name = measure_data/z
    value_name = measure_data/u
  []
[]
##--------- Outputs ------------------#

[Debug]
  show_var_residual_norms = true
[]
