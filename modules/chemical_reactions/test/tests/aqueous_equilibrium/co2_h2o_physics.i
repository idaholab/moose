# Batch CO2 - H2O equilibrium reaction at 25C
#
# Aqueous equilibrium reactions:
# a)  H+ + HCO3- = CO2(aq),         Keq = 10^(6.3447)
# b)  HCO3- = H+ + co3^2-,           Keq = 10^(-10.3288)
# c)  - H+ = OH-,                   Keq = 10^(-13.9951)
#
# The primary chemical species are h+ and hco3-, and the secondary equilibrium
# species are CO2(aq), co3^2- and OH-

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[AuxVariables]
  [ph]
  []
  [total_h+]
  []
  [total_hco3-]
  []
[]

[AuxKernels]
  [ph]
    type = PHAux
    variable = ph
    h_conc = h+
  []
  [total_h+]
    type = TotalConcentrationAux
    variable = total_h+
    primary_species = h+
    v = 'oh- co3^2- co2_aq'
    sto_v = '-1 1 1'
  []
  [total_hco3-]
    type = TotalConcentrationAux
    variable = total_hco3-
    primary_species = hco3-
    v = 'co2_aq co3^2-'
    sto_v = '1 1'
  []
[]

[Variables]
  [h+]
    initial_condition = 1e-5
  []
  [hco3-]
    initial_condition = 1e-5
  []
[]

[Physics]
  [AqueousReactionsEquilibrium/all]
    primary_species = 'hco3- h+'
    secondary_species = 'co2_aq co3^2- oh-'
    reactions = 'hco3- + h+ -> co2_aq  [log10_K=6.3447]
                 hco3- - h+ -> co3^2-  [log10_K=-10.3288]
                 - h+ -> oh-           [log10_K=-13.9951]'

    # To reproduce original input setup
    add_darcy_advection_term = true
    gravity = '0 0 0'
    pressure = ''
  []
[]

# Note that these time derivatives should not be added by the Physics
[Kernels]
  [h+_ie]
    type = PrimaryTimeDerivative
    variable = h+
  []
  [hco3-_ie]
    type = PrimaryTimeDerivative
    variable = hco3-
  []
[]

[Materials]
  [porous]
    type = GenericConstantMaterial
    prop_names = 'diffusivity porosity conductivity'
    prop_values = '1e-7 0.25 1.0'
  []
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  nl_abs_tol = 1e-12
  end_time = 1
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [h+]
    type = ElementIntegralVariablePostprocessor
    variable = h+
    execute_on = 'initial timestep_end'
  []
  [hco3-]
    type = ElementIntegralVariablePostprocessor
    variable = hco3-
    execute_on = 'initial timestep_end'
  []
  [co2_aq]
    type = ElementIntegralVariablePostprocessor
    variable = co2_aq
    execute_on = 'initial timestep_end'
  []
  [co3^2-]
    type = ElementIntegralVariablePostprocessor
    variable = co3^2-
    execute_on = 'initial timestep_end'
  []
  [oh-]
    type = ElementIntegralVariablePostprocessor
    variable = oh-
    execute_on = 'initial timestep_end'
  []
  [ph]
    type = ElementIntegralVariablePostprocessor
    variable = ph
    execute_on = 'initial timestep_end'
  []
  [total_h+]
    type = ElementIntegralVariablePostprocessor
    variable = total_h+
    execute_on = 'initial timestep_end'
  []
  [total_hco3-]
    type = ElementIntegralVariablePostprocessor
    variable = total_hco3-
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  perf_graph = true
  csv = true
[]
