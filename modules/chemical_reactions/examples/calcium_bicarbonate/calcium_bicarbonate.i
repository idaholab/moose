# Example of reactive transport model with precipitation and dissolution.
# Calcium (ca2) and bicarbonate (hco3) reaction to form calcite (CaCO3).
# Models bicarbonate injection following calcium injection, so that a
# moving reaction front forms a calcite precipitation zone. As the front moves,
# the upstream side of the front continues to form calcite via precipitation,
# while at the downstream side, dissolution of the solid calcite occurs.
#
# The reaction network considered is as follows:
# Aqueous equilibrium reactions:
# a)  h+ + hco3- = CO2(aq),             Keq = 10^(6.341)
# b)  hco3- = h+ + CO23-,               Keq = 10^(-10.325)
# c)  ca2+ + hco3- = h+ + CaCO3(aq),    Keq = 10^(-7.009)
# d)  ca2+ + hco3- = cahco3+,           Keq = 10^(-0.653)
# e)  ca2+ = h+ + CaOh+,                Keq = 10^(-12.85)
# f)  - h+ = oh-,                       Keq = 10^(-13.991)
#
# Kinetic reactions
# g)  ca2+ + hco3- = h+ + CaCO3(s),     A = 0.461 m^2/L, k = 6.456542e-2 mol/m^2 s,
#                                       Keq = 10^(1.8487)
#
# The primary chemical species are h+, hco3- and ca2+. The pressure gradient is fixed,
# and a conservative tracer is also included.
#
# This example is taken from:
# Guo et al, A parallel, fully coupled, fully implicit solution to reactive
# transport in porous media using the preconditioned Jacobian-Free Newton-Krylov
# Method, Advances in Water Resources, 53, 101-108 (2013).

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  xmax = 1
  ymax = 0.25
[]

[Variables]
  [./tracer]
  [../]
  [./ca2+]
  [../]
  [./h+]
    initial_condition = 1.0e-7
    scaling = 1e6
  [../]
  [./hco3-]
  [../]
[]

[AuxVariables]
  [./pressure]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./pressure_ic]
    type = FunctionIC
    variable = pressure
    function = pic
  [../]
  [./hco3_ic]
    type = BoundingBoxIC
    variable = hco3-
    x1 = 0.0
    y1 = 0.0
    x2 = 1.0e-10
    y2 = 0.25
    inside = 5.0e-2
    outside = 1.0e-6
  [../]
  [./ca2_ic]
    type = BoundingBoxIC
    variable = ca2+
    x1 = 0.0
    y1 = 0.0
    x2 = 1.0e-10
    y2 = 0.25
    inside = 1.0e-6
    outside = 5.0e-2
  [../]
  [./tracer_ic]
    type = BoundingBoxIC
    variable = tracer
    x1 = 0.0
    y1 = 0.0
    x2 = 1.0e-10
    y2 = 0.25
    inside = 1.0
    outside = 0.0
  [../]
[]

[Functions]
  [./pic]
    type = ParsedFunction
    expression = 60-50*x
  [../]
[]

[ReactionNetwork]
  [./AqueousEquilibriumReactions]
    primary_species = 'ca2+ hco3- h+'
    secondary_species = 'co2_aq co32- caco3_aq cahco3+ caoh+ oh-'
    pressure = pressure
    reactions = 'h+ + hco3- = co2_aq 6.341,
                 hco3- - h+ = co32- -10.325,
                 ca2+ + hco3- - h+ = caco3_aq -7.009,
                 ca2+ + hco3- = cahco3+ -0.653,
                 ca2+ - h+ = caoh+ -12.85,
                 - h+ = oh- -13.991'
  [../]
  [./SolidKineticReactions]
    primary_species = 'ca2+ hco3- h+'
    kin_reactions = 'ca2+ + hco3- - h+ = caco3_s'
    secondary_species = caco3_s
    log10_keq = 1.8487
    reference_temperature = 298.15
    system_temperature = 298.15
    gas_constant = 8.314
    specific_reactive_surface_area = 4.61e-4
    kinetic_rate_constant = 6.456542e-7
    activation_energy = 1.5e4
  [../]
[]

[Kernels]
  [./tracer_ie]
    type = PrimaryTimeDerivative
    variable = tracer
  [../]
  [./tracer_pd]
    type = PrimaryDiffusion
    variable = tracer
  [../]
  [./tracer_conv]
    type = PrimaryConvection
    variable = tracer
    p = pressure
  [../]
  [./ca2+_ie]
    type = PrimaryTimeDerivative
    variable = ca2+
  [../]
  [./ca2+_pd]
    type = PrimaryDiffusion
    variable = ca2+
  [../]
  [./ca2+_conv]
    type = PrimaryConvection
    variable = ca2+
    p = pressure
  [../]
  [./h+_ie]
    type = PrimaryTimeDerivative
    variable = h+
  [../]
  [./h+_pd]
    type = PrimaryDiffusion
    variable = h+
  [../]
  [./h+_conv]
    type = PrimaryConvection
    variable = h+
    p = pressure
  [../]
  [./hco3-_ie]
    type = PrimaryTimeDerivative
    variable = hco3-
  [../]
  [./hco3-_pd]
    type = PrimaryDiffusion
    variable = hco3-
  [../]
  [./hco3-_conv]
    type = PrimaryConvection
    variable = hco3-
    p = pressure
  [../]
[]

[BCs]
  [./tracer_left]
    type = DirichletBC
    variable = tracer
    boundary = left
    value = 1.0
  [../]
  [./tracer_right]
    type = ChemicalOutFlowBC
    variable = tracer
    boundary = right
  [../]
  [./ca2+_left]
    type = SinDirichletBC
    variable = ca2+
    boundary = left
    initial = 5.0e-2
    final = 1.0e-6
    duration = 1
  [../]
  [./ca2+_right]
    type = ChemicalOutFlowBC
    variable = ca2+
    boundary = right
  [../]
  [./hco3-_left]
    type = SinDirichletBC
    variable = hco3-
    boundary = left
    initial = 1.0e-6
    final = 5.0e-2
    duration = 1
  [../]
  [./hco3-_right]
    type = ChemicalOutFlowBC
    variable = hco3-
    boundary = right
  [../]
  [./h+_left]
    type = DirichletBC
    variable = h+
    boundary = left
    value = 1.0e-7
  [../]
  [./h+_right]
    type = ChemicalOutFlowBC
    variable = h+
    boundary = right
  [../]
[]

[Materials]
  [./porous]
    type = GenericConstantMaterial
    prop_names = 'diffusivity conductivity porosity'
    prop_values = '1e-7 2e-4 0.2'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  l_max_its = 50
  l_tol = 1e-5
  nl_max_its = 10
  nl_rel_tol = 1e-5
  end_time = 10
  [./TimeStepper]
    type = ConstantDT
    dt = 0.1
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Outputs]
  perf_graph = true
  exodus = true
[]
