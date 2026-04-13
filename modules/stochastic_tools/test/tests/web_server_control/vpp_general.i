mesh_samples = 1
mesh_outer = 3
mesh_specified=1
[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = ${fparse mesh_outer}
    nx = ${fparse max(mesh_samples,mesh_specified)}
  []
[]

[Variables]
  [temperature]
    initial_condition = 650
  []
[]
[Functions]
   [src]
      type = ParsedFunction
      expression = '3*x'
   []
[]

[Kernels]
  [heat_conduction_solid]
    type = MatDiffusion
    diffusivity = thermal_conductivity
    variable = temperature
  []
  [heat_source]
    type = BodyForce
    variable = temperature
    function = src
  []
[]

[BCs]
  [heat_DRV_outer]
    type = DirichletBC
    boundary = right
    variable = temperature
    value = 300
  []
[]

[Materials]
 [Matrix]
  type = ParsedMaterial
  property_name = thermal_conductivity
  coupled_variables = temperature
  constant_expressions = '8.314'
  constant_names = 'R'
  postprocessor_names = 'frequency_factor activation_energy'
  expression = 'frequency_factor * exp(-activation_energy / R / temperature)'
 []
[]
[Postprocessors]
  [activation_energy]
    type = ConstantPostprocessor
    value = 27700.0
  []
  [frequency_factor]
    type = ConstantPostprocessor
    value = 385478.35767
  []
[]
[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  line_search = none
  start_time = 0.0
  end_time = 100
  dt = 100
[]
[Positions]
  [SR_pos]
    type = ElementCentroidPositions
    auto_sort = True
  []
  [ring_poss]
    type = ReporterPositions
    reporters = 'SR_pos/positions_1d'
  []
[]
[VectorPostprocessors]
  [temp]
    type = PositionsFunctorValueSampler
    functors = 'temperature'
    positions = ring_poss
    sort_by = x
    execute_on = FINAL
    outputs = csv
    discontinuous = true
  []
[]
[Outputs]
  console = false
  exodus = false
[]
