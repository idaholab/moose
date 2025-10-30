[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    xmin = 0
    xmax = 1
    ny = 10
    ymin = 0
    ymax = 1
  []
  parallel_type = REPLICATED
[]

[AuxVariables]
  [params1]
    order = FIRST
    family = LAGRANGE
  []
  [params2]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [params1]
    type = ParsedAux
    variable = params1
     expression = 'x*y*t'
     use_xyzt = true
  []
    [params2]
    type = ParsedAux
    variable = params2
     expression = 'x*x*t'
     use_xyzt = true
    []
[]

[VectorPostprocessors]
  [point_sample]
    type=CSVPointValueSampler
    point_xcoord = x
    point_ycoord = y
    point_zcoord = z
    point_id = id
    points_file = 'gold/csv_point_sampler_out_point_sample_0001.csv'
    sort_by = id
    variable = 'params1 params2' 
  []
  [point_sample_x]
    type=CSVPointValueSampler
    point_xcoord = x
    point_ycoord = y
    point_zcoord = z
    point_id = id
    points_file = 'gold/csv_point_sampler_out_point_sample_0001.csv'
    sort_by = x
    variable = 'params1 params2'
  []
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]

[Executioner]
  type = Transient
  dt = 1.0
  num_steps = 2
  solve_type = NEWTON
[]

[Problem]
  solve = false
[]
