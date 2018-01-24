# Test for making sure that a coupled variable can be used inside of initQpStatefulProperties
# of a Material object.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[Variables]
  [./u]
  [../]
[]

[ICs]
  [./u_ic]
    type = ConstantIC
    value = 1.2345
    variable = u
  [../]
[]

[Materials]
  [./coupling_u]
    type = VarCouplingMaterial
    var = u
    declare_old = true
    outputs = exodus
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  exodus = true
  hide = 'u'
[]
