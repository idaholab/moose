[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
  []
[]

[GlobalParams]
  elements = 'Mo Ru'
  output_phases = 'BCCN HCPN'
  output_species = 'BCCN:Mo HCPN:Mo BCCN:Ru HCPN:Ru'
  element_potentials = 'cp:Mo cp:Ru'
[]

[ChemicalComposition]
  thermofile = Kaye_NobleMetals.dat
  initial_values = ic.csv
[]

[UserObjects]
  [data]
    type = ThermochimicaNodalData
    temperature = 2250
    elements = 'Mo Ru'
    output_phases = 'BCCN HCPN'
    execute_on = 'INITIAL TIMESTEP_END'
    reinit_requested = false # changes parallel results slightly
  []
[]

[AuxVariables]
  [n]
  []
[]

[AuxKernels]
  [thermochimica]
    type = ThermochimicaAux
    variable = n
    thermo_nodal_data_uo = data
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
