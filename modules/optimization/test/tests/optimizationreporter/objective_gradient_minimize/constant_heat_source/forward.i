[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 2
  ymax = 2
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
  []
  [./heat_source]
    type = ADMatHeatSource
    material_property = volumetric_heat
    variable = temperature
  [../]
[]

[BCs]
  [left]
    type = NeumannBC
    variable = temperature
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = temperature
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 200
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 100
  []
[]

[Functions]
  [volumetric_heat_func]
    type = ParsedFunction
    # value = 100*cos(2*pi/2*(x+1))
    value = q
    vars = 'q'
    vals = 'valueFromControl'
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
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
    variable = temperature
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
  file_base = 'forward'
[]
