[Mesh]
  type = GeneratedMesh
  nx = 2
  ny = 2
  dim = 2
[]

[Variables]
  [./u]
  [../]
  [./v]
  [../]
[]

[Preconditioning]
  [./FDP]
    type = FDP
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./conv_v]
    type = CoupledForce
    variable = v
    v = u
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = false
[]

[ICs]
  [./u]
    variable = u
    type = RandomIC
    min = 0.1
    max = 0.9
  [../]
  [./v]
    variable = v
    type = RandomIC
    min = 0.1
    max = 0.9
  [../]
[]
