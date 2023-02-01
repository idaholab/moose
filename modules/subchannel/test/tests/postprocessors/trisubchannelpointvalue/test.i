T_in = 359.15

[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = 3
    n_cells = 10
    flat_to_flat = 3.41e-2
    heated_length = 1.0
    rod_diameter = 5.84e-3
    pitch = 7.26e-3
    dwire = 1.42e-3
    hwire = 0.3048
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]

[AuxVariables]
  [T]
  []
[]

[ICs]
  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []
[]

[Problem]
  type = NoSolveProblem
[]

[Postprocessors]
  [T]
    type = SubChannelPointValue
    variable = T
    index = 0
    execute_on = 'initial timestep_end'
    height = 0.5
  []
[]

[Outputs]
  csv = true
[]

[Executioner]
  type = Transient
  nl_rel_tol = 0.9
  l_tol = 0.9
  start_time = 0.0
  end_time = 10.0
  dt = 1.0
[]
