# Additional activity coefficient in AqueousEquilibriumRxnAux AuxKernel

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [./a]
  [../]
  [./b]
  [../]
[]

[AuxVariables]
  [./c]
  [../]
  [./gamma_a]
  [../]
  [./gamma_b]
  [../]
  [./gamma_c]
  [../]
[]

[AuxKernels]
  [./c]
    type = AqueousEquilibriumRxnAux
    variable = c
    v = 'a b'
    gamma_v = 'gamma_a gamma_b gamma_c'
    sto_v = '1 1'
    log_k = 1
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
