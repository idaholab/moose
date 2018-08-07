# Explicitly adds all Kernels and AuxKernels. Used to check that the
# SolidKineticReactions parser is working correctly

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

[AuxVariables]
  [./m1]
  [../]
  [./m2]
  [../]
  [./m3]
  [../]
[]

[AuxKernels]
  [./m1]
    type = KineticDisPreConcAux
    variable = m1
    v = 'a b'
    sto_v = '1 1'
    log_k = -8
    r_area = 1
    ref_kconst = 1e-8
    e_act = 1e4
    gas_const = 8.314
    ref_temp = 298.15
    sys_temp = 298.15
  [../]
  [./m2]
    type = KineticDisPreConcAux
    variable = m2
    v = 'c d'
    sto_v = '2 3'
    log_k = -8
    r_area = 2
    ref_kconst = 2e-8
    e_act = 2e4
    gas_const = 8.314
    ref_temp = 298.15
    sys_temp = 298.15
  [../]
  [./m3]
    type = KineticDisPreConcAux
    variable = m3
    v = 'a c'
    sto_v = '1 -2'
    log_k = -8
    r_area = 3
    ref_kconst = 3e-8
    e_act = 3e4
    gas_const = 8.314
    ref_temp = 298.15
    sys_temp = 298.15
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
  [./a_kin]
    type = CoupledBEKinetic
    variable = a
    v = 'm1 m3'
    weight = '1 1'
  [../]
  [./b_kin]
    type = CoupledBEKinetic
    variable = b
    v = m1
    weight = 1
  [../]
  [./c_kin]
    type = CoupledBEKinetic
    variable = c
    v = 'm2 m3'
    weight = '2 -2'
  [../]
  [./d_kin]
    type = CoupledBEKinetic
    variable = d
    v = m2
    weight = 3
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
