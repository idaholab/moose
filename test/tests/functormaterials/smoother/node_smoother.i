[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 100
    ny = 1
    xmax = 10
    ymax = 1
  []
[]

[AuxVariables]
  [checkerboard]
  []
  [smooth]
  []
[]

[AuxKernels]
  [checker]
    type = ParsedAux
    variable = checkerboard
    # nonlinear growth to challenge the smoother a bit
    expression = '2 + x * x * sin(PI * 10 * x)'
    constant_names = 'PI'
    constant_expressions = '3.14159265359'
    use_xyzt = true
    execute_on = 'TIMESTEP_BEGIN'
  []
  [smooth]
    type = FunctorAux
    variable = smooth
    functor = 'smoothed_functor'
    execute_on = 'TIMESTEP_END'
  []
[]

[FunctorMaterials]
  [smooth]
    type = FunctorSmoother
    functors_in = 'checkerboard'
    functors_out = 'smoothed_functor'
    smoothing_technique = 'node_average'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[VectorPostprocessors]
  [line]
    type = LineValueSampler
    variable = 'smooth'
    num_points = 100
    start_point = '0.05 0.5 0'
    end_point = '9.95 0.5 0'
    sort_by = 'x'
  []
[]

[Outputs]
  exodus = true
  [out]
    type = CSV
    execute_on = 'TIMESTEP_END'
  []
[]
