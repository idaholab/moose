[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
  []
[]

[Variables]
  [eta]
  []
[]

[ChemicalComposition]
  [hcp_thermodynamics]
    elements = 'Mo Ru'
    thermodynamic_database = ../../../chemical_reactions/test/tests/thermochimica/Kaye_NobleMetals.dat
    temperature_unit = K
    pressure_unit = atm
    composition_unit = moles
    temperature = 2250
    included_phases = HCPN
    execute_on = 'INITIAL TIMESTEP_BEGIN'

    [Outputs]
      [SystemGibbsEnergies]
        [hcp_restricted_gibbs]
        []
      []
    []
  []
[]

[ICs]
  [Mo]
    type = ConstantIC
    variable = Mo
    value = 0.2
  []
  [Ru]
    type = ConstantIC
    variable = Ru
    value = 0.8
  []
  [eta]
    type = ConstantIC
    variable = eta
    value = 0.5
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = eta
  []
  [bulk]
    type = AllenCahn
    variable = eta
    f_name = free_energy
    coupled_variables = hcp_restricted_gibbs
  []
  [interface]
    type = ACInterface
    variable = eta
  []
[]

[Materials]
  [constants]
    type = GenericConstantMaterial
    prop_names = 'L kappa_op'
    prop_values = '1 0.01'
  []
  [thermochemical_free_energy]
    type = DerivativeParsedMaterial
    property_name = free_energy
    coupled_variables = 'eta hcp_restricted_gibbs'
    # eta = 0 is the reference phase and eta = 1 is the HCP-only state.
    # This scale is illustrative; physical models require consistent energy-density units.
    constant_names = 'barrier gibbs_scale'
    constant_expressions = '1 1e-6'
    expression = 'barrier*eta^2*(1-eta)^2 + gibbs_scale*hcp_restricted_gibbs*eta^2*(3-2*eta)'
    derivative_order = 2
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 2
  dt = 0.01
[]

[Postprocessors]
  [average_eta]
    type = ElementAverageValue
    variable = eta
  []
  [hcp_restricted_gibbs]
    type = NodalVariableValue
    variable = hcp_restricted_gibbs
    nodeid = 2
  []
[]

[Outputs]
  csv = true
[]
