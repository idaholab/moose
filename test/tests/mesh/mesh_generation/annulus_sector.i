# Generates a sector of an Annular Mesh between angle=Pi/4 and angle=3Pi/4
# Radius of inside circle=1
# Radius of outside circle=5
# Solves the diffusion equation with
# u=0 on inside
# u=log(5) on outside
# u=log(r) at angle=Pi/4 and angle=3Pi/4

[Mesh]
  type = AnnularMesh
  nr = 10
  nt = 12
  rmin = 1
  rmax = 5
  dmin = 45
  dmax = 135
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
  [./min_angle]
    type = FunctionDirichletBC
    variable = u
    function = 'log(sqrt(x*x + y*y))'
    boundary = 'dmin dmax'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  exodus = true
[]
