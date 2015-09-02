[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  xmax = 10
  ymax = 10
[]

[Variables]
  [./rho]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = rho
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  exodus = true
[]

[ICs]
  [./rho_IC]
    y2 = 8.75
    lc = 5
    y1 = 1.25
    x2 = 8.75
    crystal_structure = FCC
    variable = rho
    x1 = 1.25
    type = PFCFreezingIC
    min = .3
    max = .7
  [../]
[]

