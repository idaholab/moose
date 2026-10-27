[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects]
  # 'A' references 'B' and 'B' references 'A' through parameters, but neither resolves the other in
  # its constructor. This is ok (it mirrors xfem's cut_mesh2 <-> crack_tip pair): the
  # construction-order sort should currently tolerate the apparent cycle for backwards compatibility
  # and fall back to input order rather than erroring (idaholab/moose#7879).
  [A]
    type = UserObjectInterfaceTest
    uo = B
  []
  [B]
    type = UserObjectInterfaceTest
    uo = A
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
