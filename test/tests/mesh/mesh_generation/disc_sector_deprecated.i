# Generates a sector of a Disc Mesh between angle=Pi/4 and angle=3Pi/4
# Radius of outside circle=5
# Solves the diffusion equation with u=-5 at origin, and u=0 on outside
# as well as u=-5+r at angle=Pi/4 and u=-5+r^4/125 at angle=3Pi/4

[Mesh]
  type = AnnularMesh
  nr = 10
  nt = 12
  rmin = 0
  rmax = 5
  tmin = 0.785398163
  tmax = 2.356194490
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
    value = -5.0
    boundary = rmin
  [../]
  [./outer]
    type = FunctionDirichletBC
    variable = u
    function = 0
    boundary = rmax
  [../]
  [./tmin]
    type = FunctionDirichletBC
    variable = u
    function = '-5.0+sqrt(x*x + y*y)'
    boundary = tmin
  [../]
  [./tmax]
    type = FunctionDirichletBC
    variable = u
    function = '-5.0+pow(x*x + y*y, 2)/125'
    boundary = tmax
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  exodus = true
[]
