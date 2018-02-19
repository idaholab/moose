# Missing stoichiometric coefficient in AqueousEquilibriumRxnAux AuxKernel
# Simple reaction-diffusion example without using the action.
# In this example, two primary species a and b diffuse towards each other from
# opposite ends of a porous medium, reacting when they meet to form a mineral
# precipitate
# This simulation is identical to 2species.i, but explicitly includes the AuxVariables,
# AuxKernels, and Kernels that the action in 2species.i adds

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
  [./mineral]
  [../]
[]

[AuxKernels]
  [./mineral_conc]
    type = KineticDisPreConcAux
    variable = mineral
    sto_v = 1
    v = 'a b'
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
