[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  xmax = 15
  ymax = 15
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
    y2 = 12.5
    lc = 5
    y1 = 2.5
    x2 = 12.5
    crystal_structure = BCC
    variable = rho
    x1 = 2.5
    type = PFCFreezingIC
    min = .3
    max = .7
  [../]
[]

