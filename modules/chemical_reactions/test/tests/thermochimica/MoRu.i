[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 1
  []
[]

[GlobalParams]
  elements = 'Mo Ru'
  output_phases = 'BCCN HCPN'
  output_species = 'BCCN:Mo HCPN:Mo BCCN:Ru HCPN:Ru'
  element_potentials = 'cp:Mo cp:Ru'
  output_vapor_pressures = 'gas_ideal:Mo'
[]

[ChemicalComposition]
  thermofile = Kaye_NobleMetals.dat
  tunit = K
  punit = atm
  munit = moles
[]

[ICs]
  [Mo]
    type = FunctionIC
    variable = Mo
    function = '0.8*(1-x)+4.3*x'
  []
  [Ru]
    type = FunctionIC
    variable = Ru
    function = '0.2*(1-x)+4.5*x'
  []
[]

[UserObjects]
  [data]
    type = ThermochimicaNodalData
    temperature = 2250
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

[VectorPostprocessors]
  [NodalData]
    type = NodalValueSampler
    variable = 'Mo Ru BCCN:Mo HCPN:Mo BCCN:Ru HCPN:Ru cp:Mo cp:Ru'
    execute_on = TIMESTEP_END
    sort_by = id
  []
[]

[Outputs]
  exodus = true
[]
