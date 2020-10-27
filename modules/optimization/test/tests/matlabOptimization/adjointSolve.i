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
    type = VectorPostprocessorPointSource
    variable = 'temperature'
    value_name = 'value'
    vector_postprocessor = 'csv_reader'
    # make this take a vpp there is csv reader vpp so try having that vpp read in the csv instead of this and apply here.
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
  [csv_reader]
    type = CSVReader
    csv_file = 'zAdjointInput/inputForces.csv'
    execute_on = initial
  []
[]

[Outputs]
  [exodus]
    file_base = 'zAdjointOutput/out'
    type = Exodus
    execute_on = final
  []
  [fullResponseVector]
    file_base = 'zAdjointOutput/all'
    type = CSV
    execute_vector_postprocessors_on = final
  []
[]
