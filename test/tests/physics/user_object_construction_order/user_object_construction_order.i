[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

# The Physics is declared before the [UserObjects] block it depends on. The Physics adds a
# UserObject whose constructor accesses 'dep_uo', so 'dep_uo' must be constructed first regardless
# of input order (idaholab/moose#7879).
[Physics]
  [Test]
    [AddUserObject]
      [physics]
        dependency = dep_uo
      []
    []
  []
[]

[UserObjects]
  [dep_uo]
    type = UserObjectInterfaceTest
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
