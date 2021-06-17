###########################################################
# This is a test of the UserObject System. The
# LayeredIntegral UserObject executes independently during
# the solve to compute a user-defined value. In this case
# an integral value in discrete layers along a vector
# in the domain. (Type: ElementalUserObject)
#
# @Requirement F6.40
###########################################################


[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 6
  nz = 6
[]

[Variables]
  [./u]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  [../]
[]

[AuxVariables]
  [./layered_integral]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[FVKernels]
  [./diff]
    type = FVDiffusion
    variable = u
    coeff = 1
  [../]
[]

[AuxKernels]
  [./liaux]
    type = SpatialUserObjectAux
    variable = layered_integral
    execute_on = timestep_end
    user_object = layered_integral
  [../]
[]

[FVBCs]
  [./bottom]
    type = FVDirichletBC
    variable = u
    boundary = bottom
    value = 0
  [../]
  [./top]
    type = FVDirichletBC
    variable = u
    boundary = top
    value = 1
  [../]
[]

[UserObjects]
  [./layered_integral]
    type = LayeredIntegral
    direction = y
    num_layers = 3
    variable = u
    execute_on = linear
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = fv_out
  exodus = true
[]
