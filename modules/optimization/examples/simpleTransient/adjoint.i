[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [dt]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [dirichlet]
    type = DirichletBC
    variable = u
    boundary = 'left right top bottom'
    value = 0
  []
[]

[Reporters]
  [measured_data]
    type = OptimizationData
    measurement_file = mms_data.csv
    file_xcoord = x
    file_ycoord = y
    file_zcoord = z
    file_time = t
    file_value = u
  []
[]

[DiracKernels]
  [misfit]
    type = VectorPointSource
    variable = u
    value = measured_data/misfit_values
    coord_x = measured_data/measurement_xcoord
    coord_y = measured_data/measurement_ycoord
    coord_z = measured_data/measurement_zcoord
    time = measured_data/measurement_time
    reverse_time_end = 1
  []
[]

[VectorPostprocessors]
  [src_values]
    type = CSVReader
    csv_file = source_params.csv
    header = true
  []
[]

[Functions]
  [source]
    type = VectorNearestPointFunction
    coord_x = src_values/coordx
    coord_y = src_values/coordy
    time = src_values/time
    value = src_values/values
  []
[]

[VectorPostprocessors]
  [adjoint]
    type = ElementOptimizationSourceFunctionInnerProduct
    variable = u
    function = source
    reverse_time_end = 1
  []
[]

[Executioner]
  type = Transient

  num_steps = 100
  end_time = 1

  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  console = false
[]
