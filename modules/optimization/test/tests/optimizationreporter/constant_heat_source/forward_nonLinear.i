[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 2
  ymax = 2
[]

[Variables]
  [T]
    initial_condition = 100
  []
[]

[Kernels]
  [heat_conduction]
    type = MatDiffusion
    variable = T
    diffusivity = thermal_conductivity
  []
  [heat_source]
    type = BodyForce
    function = volumetric_heat_func
    variable = T
  []
[]

[BCs]
  [left]
    type = NeumannBC
    variable = T
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = T
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = T
    boundary = bottom
    value = 200
  []
  [top]
    type = DirichletBC
    variable = T
    boundary = top
    value = 100
  []
[]

[Functions]
  [volumetric_heat_func]
    type = ParsedOptimizationFunction
    expression = q
    param_symbol_names = 'q'
    param_vector_name = 'params/q'
  []
[]

[Materials]
  [steel]
    type = ParsedMaterial
    f_name = 'thermal_conductivity'
    function = '.01*T'
    args = 'T'
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'preonly lu       superlu_dist'
[]

[Reporters]
  [measure_data]
    type = OptimizationData
    variable = T
  []
  [params]
    type = ConstantReporter
    real_vector_names = 'q'
    real_vector_values = '0' # Dummy value
  []
[]

[Outputs]
  console = false
  file_base = 'forward_nl'
[]
