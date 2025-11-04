[GlobalParams]
  use_displaced_mesh = true
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1
    xmax = 1
  []
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [something]
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxKernels]
  [something]
    type = ConstantAux
    variable = something
    value = 7
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Postprocessors]
  [cell_t_left]
    type = PointValue
    variable = something
    point = '-0.01 0.0 0.0'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
[]
