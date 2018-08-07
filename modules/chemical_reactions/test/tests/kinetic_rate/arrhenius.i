# Check the correct temperature dependence of the kinetic rate constant using
# the Arrhenius equation. Two kinetic reactions take place at different system
# temperatures. The Arrhenius equation states that the kinetic rate increases
# with temperature, so more mineral should be precipitated for the higher system
# temperature. In this case, the AuxVariables kinetic_rate1 and mineral1 should
# be larger than kinetic_rate0 and mineral0.

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [./a0]
    initial_condition = 0.1
  [../]
  [./b0]
    initial_condition = 0.1
  [../]
  [./a1]
    initial_condition = 0.1
  [../]
  [./b1]
    initial_condition = 0.1
  [../]
[]

[AuxVariables]
  [./mineral0]
  [../]
  [./mineral1]
  [../]
  [./kinetic_rate0]
  [../]
  [./kinetic_rate1]
  [../]
[]

[AuxKernels]
  [./kinetic_rate0]
    type = KineticDisPreRateAux
    variable = kinetic_rate0
    e_act = 1.5e4
    r_area = 1
    log_k = -6
    ref_kconst = 1e-8
    gas_const = 8.31434
    ref_temp = 298.15
    sys_temp = 298.15
    sto_v = '1 1'
    v = 'a0 b0'
  [../]
  [./kinetic_rate1]
    type = KineticDisPreRateAux
    variable = kinetic_rate1
    e_act = 1.5e4
    r_area = 1
    log_k = -6
    ref_kconst = 1e-8
    gas_const = 8.31434
    ref_temp = 298.15
    sys_temp = 323.15
    sto_v = '1 1'
    v = 'a1 b1'
  [../]
  [./mineral0_conc]
    type = KineticDisPreConcAux
    variable = mineral0
    e_act = 1.5e4
    r_area = 1
    log_k = -6
    ref_kconst = 1e-8
    gas_const = 8.31434
    ref_temp = 298.15
    sys_temp = 298.15
    sto_v = '1 1'
    v = 'a0 b0'
  [../]
  [./mineral1_conc]
    type = KineticDisPreConcAux
    variable = mineral1
    e_act = 1.5e4
    r_area = 1
    log_k = -6
    ref_kconst = 1e-8
    gas_const = 8.31434
    ref_temp = 298.15
    sys_temp = 323.15
    sto_v = '1 1'
    v = 'a1 b1'
  [../]
[]

[Kernels]
  [./a0_ie]
    type = PrimaryTimeDerivative
    variable = a0
  [../]
  [./b0_ie]
    type = PrimaryTimeDerivative
    variable = b0
  [../]
  [./a0_r]
    type = CoupledBEKinetic
    variable = a0
    v = mineral0
    weight = 1
  [../]
  [./b0_r]
    type = CoupledBEKinetic
    variable = b0
    v = mineral0
    weight = 1
  [../]
  [./a1_ie]
    type = PrimaryTimeDerivative
    variable = a1
  [../]
  [./b1_ie]
    type = PrimaryTimeDerivative
    variable = b1
  [../]
  [./a1_r]
    type = CoupledBEKinetic
    variable = a1
    v = mineral1
    weight = 1
  [../]
  [./b1_r]
    type = CoupledBEKinetic
    variable = b1
    v = mineral1
    weight = 1
  [../]
[]

[Materials]
  [./porous]
    type = GenericConstantMaterial
    prop_names = porosity
    prop_values = 0.2
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  end_time = 1
  dt = 1
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Outputs]
  exodus = true
  perf_graph = true
  print_linear_residuals = true
[]
