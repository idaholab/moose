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
    type = MooseVariableFVReal
  []
  [smooth]
    type = MooseVariableFVReal
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
    type = FunctorElementalAux
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
    # Using the face values will not smooth a checkerboard because the 'extreme' neighbor value are
    # mixed with the element value
    # Using the layered element average will smooth a checkerboard in 2D inside the volume, and fail to do so
    # near the boundaries. In 1D it wont fix a checkboard as it does not average with the local value
    # smoothing_technique = 'layered_elem_average'
    smoothing_technique = 'remove_checkerboard'
    # smoothing_technique = 'face_average'
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
