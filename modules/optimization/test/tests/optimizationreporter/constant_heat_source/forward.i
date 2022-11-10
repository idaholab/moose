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
    type = MatDiffusion
    variable = temperature
    diffusivity = thermal_conductivity
  []
  [heat_source]
    type = BodyForce
    function = volumetric_heat_func
    variable = temperature
  []
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
    value = q
    vars = 'q'
    vals = 'valueFromControl'
  []
[]

[Materials]
  [steel]
    type = GenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
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
    type = OptimizationData
  []
[]

[Outputs]
  console = false
  file_base = 'forward'
[]
