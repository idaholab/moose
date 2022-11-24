[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Preconditioning/pbp]
  type = PBP
  solve_order = 'vars0 vars1 vars2 vars3 vars4 vars5 vars6 vars7 vars8 vars9'
  preconditioner = 'AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG'
[]

[Executioner]
  type = Steady
  solve_type = JFNK
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]

[Testing/LotsOfDiffusion/vars]
  number = 10
  diffusion_coefficients = '1 1'
[]
