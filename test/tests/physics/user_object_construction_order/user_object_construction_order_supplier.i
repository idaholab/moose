[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

# The [UserObjects] block is declared before the Physics that supplies the UserObject it depends on.
# 'consumer' accesses 'physics_uo' in its constructor, and 'physics_uo' is added by the Physics, so
# the Physics must add it first regardless of input file order (idaholab/moose#7879).
[UserObjects]
  [consumer]
    type = UserObjectInterfaceTest
    uo = physics_uo
    has = true
  []
[]

[Physics]
  [Test]
    [AddUserObject]
      [physics]
      []
    []
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
