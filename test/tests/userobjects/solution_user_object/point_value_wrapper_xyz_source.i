[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 2
    ny = 1
  []
[]

[AuxVariables]
  [source_xyz]
    family = XYZ
    order = CONSTANT
  []
[]

[AuxKernels]
  [set_source_xyz]
    type = ParsedAux
    variable = source_xyz
    expression = 'if(x < 0.5, 3, 5)'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Problem]
  kernel_coverage_check = false
  skip_nl_system_check = true
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1
[]

[Outputs]
  file_base = point_value_wrapper_xyz_source
  xda = true
[]
