[Mesh]
  type = QuadSubChannelMesh
  nx = 6
  ny = 6
  max_dz = 1.0
  pitch = 0.0126
  rod_diameter = 0.00950
  gap = 0.00095 # the half gap between sub-channel assemblies
  heated_length = 1.0
  spacer_z = '0.0'
  spacer_k = '0.0'
[]

[AuxVariables]
  [mdot]
  []
[]

[AuxKernels]
  [mdot_uniform]
    type = UniformlyDistributedMassFlowRateAux
    variable = mdot
    boundary = inlet
    mass_flow = 36.0 #kg/sec
    execute_on = 'initial'
  []
[]

[Postprocessors]
  [total_mass_flow_rate]
    type = NodalSum
    boundary = inlet
    variable = mdot
    execute_on = 'initial'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
