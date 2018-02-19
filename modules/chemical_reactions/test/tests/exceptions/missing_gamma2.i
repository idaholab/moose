# Missing activity coefficient in CoupledBEEquilibriumSub Kernel

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [./a]
  [../]
  [./b]
  [../]
  [./c]
  [../]
[]

[AuxVariables]
  [./gamma_a]
  [../]
  [./gamma_b]
  [../]
  [./gamma_c]
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
  [./aeq]
    type = CoupledBEEquilibriumSub
    variable = a
    log_k = 1
    weight = 2
    sto_u = 2
    v = 'b c'
    sto_v = '1 1'
    gamma_v = gamma_b
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
  end_time = 1
[]
