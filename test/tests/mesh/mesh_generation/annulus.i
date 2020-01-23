# Generates an Annular Mesh
# Radius of inside circle=1
# Radius of outside circle=5
# Solves the diffusion equation with
# u=0 on inside
# u=log(5) on outside

[Mesh]
  type = AnnularMesh
  nr = 10
  nt = 12
  rmin = 1
  rmax = 5
  growth_r = 1.3
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./inner]
    type = DirichletBC
    variable = u
    value = 0.0
    boundary = rmin
  [../]
  [./outer]
    type = FunctionDirichletBC
    variable = u
    function = log(5)
    boundary = rmax
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  exodus = true
[]
