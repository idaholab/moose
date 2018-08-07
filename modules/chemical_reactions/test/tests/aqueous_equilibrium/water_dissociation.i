# Dissociation of H2O at 25C
# The dissociation of water into H+ and OH- is given by
# the equilibrium reaction H20 = H+ + OH-
#
# This can be entered in the ReactionNetwork block using
# Aqueous equilibrium reaction: - H+ = OH-, Keq = 10^(-13.9951)
#
# Note that H2O does not need to be explicitly included.
#
# The primary chemical species is H+, and the secondary equilibrium
# species is OH-.
#
# The initial concentration of H+ is 10^-7, which is its value in neutral
# water. The pH of this water is therefore 7.

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[AuxVariables]
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

[Variables]
  [./h+]
    initial_condition = 1.0e-7
  [../]
[]

[ReactionNetwork]
  [./AqueousEquilibriumReactions]
    primary_species = h+
    secondary_species = oh-
    reactions = '- h+ = oh- -13.9951'
  [../]
[]

[Kernels]
  [./h+_ie]
    type = PrimaryTimeDerivative
    variable = h+
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
  end_time = 1
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
[]

[Outputs]
  perf_graph = true
  csv = true
[]
