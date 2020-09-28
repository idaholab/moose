[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1.0
    ymin = 0
    ymax = 2.0
    elem_type = QUAD4
    nx = 10
    ny = 20
  []
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
[]

[DiracKernels]
  [force_0]
    type = ConstantPointSource
    variable = temperature
    position_value_file=zForwardInput/inputForces.csv
  []
[]

[BCs]
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 0
  []
  [left]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = temperature
    boundary = right
    value = 0
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = .5
  []
[]

[Problem]
  type = FEProblem
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[VectorPostprocessors]
  [temperatures]
    type = NodalValueSampler
    variable = temperature
    block = '0'
    sort_by = id
    outputs = fullResponseVector
  []
[]

[Outputs]
  [exodus]
    file_base = 'zForwardOutput/out'
    type = Exodus
    execute_on = final
  []
  [fullResponseVector]
    file_base = 'zForwardOutput/all'
    type = CSV
    execute_vector_postprocessors_on = final
  []
[]
