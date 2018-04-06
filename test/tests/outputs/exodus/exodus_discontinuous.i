##
# \file exodus/exodus_discontinuous.i
# \example exodus/exodus_discontinuous.i
# Input file for testing discontinuous data output
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[AuxVariables]
  [./disc_u]
    family = monomial
    order = constant
    [./InitialCondition]
      type = RandomIC
    [../]
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Problem]
  solve = false
  type = FEProblem
[]

[Outputs]
  execute_on = 'timestep_end'
  [./out]
    type = Exodus
    discontinuous = true
    elemental_as_nodal = true
  [../]
[]
