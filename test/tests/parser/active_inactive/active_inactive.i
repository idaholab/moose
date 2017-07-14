#############################################################
# This input file demonstrates the use of the active/inactive
# block level parameters that can be used to toggle individual
# blocks on/off for every block in a MOOSE-based input file.
#
# "active" and "inactive" cannot be used within the same block
##############################################################

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  active = 'u'

  [./u]
  [../]
[]

[AuxVariables]
  inactive = 'aux1 aux3'

  # The parameters in the inactive sections can be invalid because
  # they are never parsed.
  [./aux1]
    type = DoesntExist
    flintstones = 'fred wilma'
  [../]
  [./aux2]
  [../]
  [./aux3]
    order = TENZILLION
  [../]
  [./aux4]
  [../]
[]

[AuxKernels]
  active = 'aux2 aux4'

  # You can use active or inactive depending on whatever is easier
  [./aux1]
    type = ConstantAux
    value = 1
    variable = aux1
  [../]
  [./aux2]
    type = ConstantAux
    value = 2
    variable = aux2
  [../]
  [./aux3]
    type = ConstantAux
    value = 3
    variable = aux3
  [../]
  [./aux4]
    type = ConstantAux
    value = 4
    variable = aux4
  [../]
[]

[Kernels]
  inactive = ''

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  inactive = ''
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  inactive = Adaptivity
  [./Adaptivity]
  [../]
[]

# No output so we can override several parameters and test them concurrently
