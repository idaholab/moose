# Example of batch reaction of calcium (Ca++) and bicarbonate (HCO3-) precipitation
# to form calcite (CaCO3).
#
# The reaction network considered is as follows:
# Aqueous equilibrium reactions:
# a)  H+ + HCO3- = CO2(aq),             Keq = 10^(6.341)
# b)  HCO3- = H+ + CO3--,               Keq = 10^(-10.325)
# c)  Ca++ + HCO3- = H+ + CaCO3(aq),    Keq = 10^(-7.009)
# d)  Ca++ + HCO3- = CaHCO3+,           Keq = 10^(-0.653)
# e)  Ca++ = H+ + CaOh+,                Keq = 10^(-12.85)
# f)  - H+ = OH-,                       Keq = 10^(-13.991)
#
# Kinetic reactions
# g)  Ca++ + HCO3- = H+ + CaCO3(s),     A = 0.461 m^2/L, k = 6.456542e-2 mol/m^2 s,
#                                       Keq = 10^(1.8487)
#
# The primary chemical species are H+, HCO3- and Ca++.

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [./ca++]
    initial_condition = 2.0e-2
  [../]
  [./h+]
    initial_condition = 1.0e-8
  [../]
  [./hco3-]
    initial_condition = 1.0e-2
  [../]
[]

[AuxVariables]
  [./caco3_s]
  [../]
  [./ph]
  [../]
[]

[AuxKernels]
  [./ph]
    type = PHAux
    h_conc = h+
    variable = ph
  [../]
[]

[ReactionNetwork]
  [./AqueousEquilibriumReactions]
    primary_species = 'ca++ hco3- h+'
    secondary_species = 'co2_aq co3-- caco3_aq cahco3+ caoh+ oh-'
    reactions = 'h+ + hco3- = co2_aq 6.3447,
                 hco3- - h+ = co3-- -10.3288,
                 ca++ + hco3- - h+ = caco3_aq -7.0017,
                 ca++ + hco3- = cahco3+ -1.0467,
                 ca++ - h+ = caoh+ -12.85,
                 - h+ = oh- -13.9951'
  [../]
  [./SolidKineticReactions]
    primary_species = 'ca++ hco3- h+'
    kin_reactions = 'ca++ + hco3- - h+ = caco3_s'
    secondary_species = caco3_s
    log10_keq = 1.8487
    reference_temperature = 298.15
    system_temperature = 298.15
    gas_constant = 8.314
    specific_reactive_surface_area = 0.1
    kinetic_rate_constant = 1e-6
    activation_energy = 1.5e4
  [../]
[]

[Kernels]
  [./ca++_ie]
    type = PrimaryTimeDerivative
    variable = ca++
  [../]
  [./h+_ie]
    type = PrimaryTimeDerivative
    variable = h+
  [../]
  [./hco3-_ie]
    type = PrimaryTimeDerivative
    variable = hco3-
  [../]
[]

[Materials]
  [./porous]
    type = GenericConstantMaterial
    prop_names = 'porosity diffusivity'
    prop_values = '0.25 1e-9'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  end_time = 100
  dt = 10
  nl_abs_tol = 1e-12
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Postprocessors]
  [./h+]
    type = ElementIntegralVariablePostprocessor
    variable = h+
    execute_on = 'initial timestep_end'
  [../]
  [./ca++]
    type = ElementIntegralVariablePostprocessor
    variable = ca++
    execute_on = 'initial timestep_end'
  [../]
  [./hco3-]
    type = ElementIntegralVariablePostprocessor
    variable = hco3-
    execute_on = 'initial timestep_end'
  [../]
  [./co2_aq]
    type = ElementIntegralVariablePostprocessor
    variable = co2_aq
    execute_on = 'initial timestep_end'
  [../]
  [./oh-]
    type = ElementIntegralVariablePostprocessor
    variable = oh-
    execute_on = 'initial timestep_end'
  [../]
  [./co3--]
    type = ElementIntegralVariablePostprocessor
    variable = co3--
    execute_on = 'initial timestep_end'
  [../]
  [./caco3_aq]
    type = ElementIntegralVariablePostprocessor
    variable = caco3_aq
    execute_on = 'initial timestep_end'
  [../]
  [./caco3_s]
    type = ElementIntegralVariablePostprocessor
    variable = caco3_s
    execute_on = 'initial timestep_end'
  [../]
  [./ph]
    type = ElementIntegralVariablePostprocessor
    variable = ph
    execute_on = 'initial timestep_end'
  [../]
  [./calcite_vf]
    type = TotalMineralVolumeFraction
    variable = caco3_s
    molar_volume = 36.934e-6
  [../]
[]

[Outputs]
  perf_graph = true
  csv = true
[]
