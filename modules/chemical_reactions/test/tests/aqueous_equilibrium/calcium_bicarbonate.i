# Calcium (Ca++) and bicarbonate (HCO3-) batch equilibrium reaction at 25C
#
# Aqueous equilibrium reactions:
# a)  H+ + HCO3- = CO2(aq),          Keq = 10^(6.3447)
# b)  HCO3- = H+ + CO3--,            Keq = 10^(-10.3288)
# c)  Ca++ + HCO3- = H+ + CaCO3(aq), Keq = 10^(-7.0017)
# d)  Ca++ + HCO3- = CaHCO3+,        Keq = 10^(1.0467)
# e)  Ca++ = H+ + CaOH+,             Keq = 10^(-12.85)
# c)  - H+ = OH-,                    Keq = 10^(-13.9951)
# d)
#
# The primary chemical species are Ca++, H+ and HCO3-, and the secondary equilibrium
# species are CO2(aq), CO3--, CaCO3(aq), CaHCO3+, CaOH+ and OH-

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[AuxVariables]
  [./ph]
  [../]
  [./total_ca++]
  [../]
  [./total_h+]
  [../]
  [./total_hco3-]
  [../]
[]

[AuxKernels]
  [./ph]
    type = PHAux
    variable = ph
    h_conc = h+
  [../]
  [./total_ca++]
    type = TotalConcentrationAux
    variable = total_ca++
    primary_species = ca++
    v = 'caco3_aq cahco3+ caoh+'
    sto_v = '1 1 1'
  [../]
  [./total_h+]
    type = TotalConcentrationAux
    variable = total_h+
    primary_species = h+
    v = 'co2_aq co3-- caco3_aq oh-'
    sto_v = '1 -1 -1 -1'
  [../]
  [./total_hco3-]
    type = TotalConcentrationAux
    variable = total_hco3-
    primary_species = hco3-
    v = 'co2_aq co3-- caco3_aq cahco3+'
    sto_v = '1 1 1 1'
  [../]
[]

[Variables]
  [./ca++]
    initial_condition = 1.0e-5
  [../]
  [./h+]
    initial_condition = 1.0e-5
  [../]
  [./hco3-]
    initial_condition = 3.0e-5
  [../]
[]

[ReactionNetwork]
  [./AqueousEquilibriumReactions]
    primary_species = 'ca++ hco3- h+'
    secondary_species = 'co2_aq co3-- caco3_aq cahco3+ caoh+ oh-'
    reactions = 'h+ + hco3- = co2_aq 6.3447,
                 hco3- - h+ = co3-- -10.3288,
                 ca++ + hco3- - h+ = caco3_aq -7.0017,
                 ca++ + hco3- = cahco3+ 1.0467,
                 ca++ - h+ = caoh+ -12.85,
                 - h+ = oh- -13.9951'
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
    prop_names = 'diffusivity porosity'
    prop_values = '1e-7 0.25'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  nl_abs_tol = 1e-12
  end_time = 1
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Postprocessors]
  [./ca++]
    type = ElementIntegralVariablePostprocessor
    variable = ca++
    execute_on = 'initial timestep_end'
  [../]
  [./h+]
    type = ElementIntegralVariablePostprocessor
    variable = h+
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
  [./cahco3+]
    type = ElementIntegralVariablePostprocessor
    variable = cahco3+
    execute_on = 'initial timestep_end'
  [../]
  [./caoh+]
    type = ElementIntegralVariablePostprocessor
    variable = caoh+
    execute_on = 'initial timestep_end'
  [../]
  [./oh-]
    type = ElementIntegralVariablePostprocessor
    variable = oh-
    execute_on = 'initial timestep_end'
  [../]
  [./ph]
    type = ElementIntegralVariablePostprocessor
    variable = ph
    execute_on = 'initial timestep_end'
  [../]
  [./total_ca++]
    type = ElementIntegralVariablePostprocessor
    variable = total_ca++
    execute_on = 'initial timestep_end'
  [../]
  [./total_hco3-]
    type = ElementIntegralVariablePostprocessor
    variable = total_hco3-
    execute_on = 'initial timestep_end'
  [../]
  [./total_h+]
    type = ElementIntegralVariablePostprocessor
    variable = total_h+
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  perf_graph = true
  csv = true
[]
