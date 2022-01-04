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
    type = ADHeatConduction
    thermal_conductivity = 'thermal_conductivity'
    variable = T
  []
  [./heat_source]
    type = ADMatHeatSource
    material_property = 'volumetric_heat'
    variable = T
  [../]
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
    type = ParsedFunction
    value = q
    vars = 'q'
    # vals = # This is set by the master app OptimizationMultiAppCommandLineControl
  []
[]

[Materials]
  [steel]
    type = ADParsedMaterial
    f_name = 'thermal_conductivity'
    function = '.01*T'
    args = 'T'
    #outputs = exodus
  []
  [volumetric_heat]
    type = ADGenericFunctionMaterial
    prop_names = 'volumetric_heat'
    prop_values = 'volumetric_heat_func'
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

[VectorPostprocessors]
  [data_pt]
    type = VppPointValueSampler
    variable = T
    reporter_name = measure_data
    outputs = none
  []
[]

[Reporters]
  [measure_data]
    type=OptimizationData
  []
[]

[Outputs]
  # console = true
  # exodus = true
  # csv = true
  file_base = 'forward_nl'
[]
