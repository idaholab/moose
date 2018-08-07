# Test SolidKineticReactions parser

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [./a]
    initial_condition = 0.1
  [../]
  [./b]
    initial_condition = 0.1
  [../]
  [./c]
    initial_condition = 0.1
  [../]
  [./d]
    initial_condition = 0.1
  [../]
[]

[ReactionNetwork]
  [./SolidKineticReactions]
    primary_species = 'a b c d'
    secondary_species = 'm1 m2 m3'
    kin_reactions = '(1.0)a + (1.0)b = m1,
                      2c + 3d = m2,
                      a - 2c = m3'
    log10_keq = '-8 -8 -8'
    specific_reactive_surface_area = '1 2 3'
    kinetic_rate_constant = '1e-8 2e-8 3e-8'
    activation_energy = '1e4 2e4 3e4'
    gas_constant = 8.314
    reference_temperature = '298.15 298.15 298.15'
    system_temperature = '298.15 298.15 298.15'
  [../]
[]

[Kernels]
  [./a_ie]
    type = PrimaryTimeDerivative
    variable = a
  [../]
  [./b_ie]
    type = PrimaryTimeDerivative
    variable = b
  [../]
  [./c_ie]
    type = PrimaryTimeDerivative
    variable = c
  [../]
  [./d_ie]
    type = PrimaryTimeDerivative
    variable = d
  [../]
[]

[Materials]
  [./porous]
    type = GenericConstantMaterial
    prop_names = porosity
    prop_values = 0.1
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  end_time = 1
  l_tol = 1e-10
  nl_rel_tol = 1e-10
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Outputs]
  file_base = kinetic_out
  exodus = true
  perf_graph = true
  print_linear_residuals = true
[]
