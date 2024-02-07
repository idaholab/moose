[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    subdomain_ids = '0 1 2 3'
  []
[]

[Problem]
  type = CheckActiveMatPropProblem
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[Kernels]
  [diff1]
    type = DiffMKernel
    variable = u
    mat_prop = diff1
  []
  [diff2]
    type = DiffMKernel
    variable = v
    mat_prop = diff2
  []
[]

[BCs]
  [left_u]
    type = MTBC
    variable = u
    boundary = 3
    grad = 4
    prop_name = c1
  []
  [right_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  []
  [left_v]
    type = DirichletBC
    variable = v
    boundary = 3
    value = 1
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 0
  []
[]

[Materials]
# There are total five material properties 'diff1', 'diff2', 'r1', 'r2', 'c1' get evaluated in the following loops:
#  E1 - NonlinearSystemBase::computeResidualTags
#  E2 - NonlinearSystemBase::computeJacobianTags
#  E3 - AuxiliarySystem::compute(linear)
#  E4 - AuxiliarySystem::compute(timestep_end)
#  E5 - FEProblemBase::computeUserObjects(timestep_end)
#
# Following tables show when and where (blocks or boundaries) the material properties are active.
#
# diff1 (by kernel 'diff1' and BC 'left_u'):
#         | E1 E2 E3 E4 E5
#      -------------------
#       0 |  x  x
#       1 |  x  x
#       2 |  x  x
#       3 |  x  x
#      b3 |  x  x
#
# diff2 (by kernel 'diff2' and BC 'left_u'):
#         | E1 E2 E3 E4 E5
#      -------------------
#       0 |  x  x
#       1 |  x  x
#       2 |  x  x
#       3 |  x  x
#      b3 |  x  x
#
# r1 (by auxkernel 'r1' and postprocessor 'r1'):
#         | E1 E2 E3 E4 E5
#      -------------------
#       0 |        x
#       1 |        x     x
#       2 |
#       3 |
#      b3 |
#
# r2 (by auxkernel 'r2' and postprocessor 'r2'):
#         | E1 E2 E3 E4 E5
#      -------------------
#       0 |
#       1 |           x
#       2 |           x  x
#       3 |
#      b3 |
#
# c1 (by BC 'left_u'):
#         | E1 E2 E3 E4 E5
#      -------------------
#       0 |
#       1 |
#       2 |
#       3 |
#      b3 |  x  x
#

  [dm]
    type = ActiveGenericConstantMaterial
    prop_names  = 'diff1 diff2 r1 r2'
    prop_values = '2 3 4 5'
  []
  [bnd]
    type = ActiveGenericConstantMaterial
    boundary = 3
    prop_names  = 'c1'
    prop_values = '2'
  []
[]

[AuxVariables]
  [r1_linear]
    family = MONOMIAL
    order = CONSTANT
  []
  [r2_linear]
    family = MONOMIAL
    order = CONSTANT
  []
  [r1_timestep_end]
    family = MONOMIAL
    order = CONSTANT
  []
  [r2_timestep_end]
    family = MONOMIAL
    order = CONSTANT
  []
  [r1]
    family = MONOMIAL
    order = CONSTANT
  []
  [r2]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [r1_linear]
    type = CheckActiveMatProp
    variable = r1_linear
    execute_on = linear
    prop_name = r1
  []
  [r1_timestep_end]
    type = CheckActiveMatProp
    variable = r1_timestep_end
    execute_on = timestep_end
    prop_name = r1
  []
  [r2_linear]
    type = CheckActiveMatProp
    variable = r2_linear
    execute_on = linear
    prop_name = r2
  []
  [r2_timestep_end]
    type = CheckActiveMatProp
    variable = r2_timestep_end
    execute_on = timestep_end
    prop_name = r2
  []
  [r1]
    type = MaterialRealAux
    variable = r1
    block = '0 1'
    property = r1
    execute_on = 'linear'
  []
  [r2]
    type = MaterialRealAux
    variable = r2
    block = '1 2'
    property = r2
    execute_on = 'timestep_end'
  []
[]

[Postprocessors]
  [r1]
    type = ElementIntegralMaterialProperty
    block = 1
    mat_prop = r1
  []
  [r2]
    type = ElementIntegralMaterialProperty
    block = 2
    mat_prop = r2
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
