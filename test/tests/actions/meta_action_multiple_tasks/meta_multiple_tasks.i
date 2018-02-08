[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

# An Action that adds an Action (AddMatAndKernel) that satisfies multiple tasks!
[MetaMultipleTasks]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  exodus = true
[]
