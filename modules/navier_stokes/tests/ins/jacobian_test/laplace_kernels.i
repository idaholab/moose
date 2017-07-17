# This input file tests Dirichlet pressure in/outflow boundary conditions for the incompressible NS equations.
[GlobalParams]
  gravity = '0 0 0'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 3.0
  ymin = 0
  ymax = 1.0
  nx = 1
  ny = 1
  elem_type = QUAD9
[]

[Variables]
  [./vel_x]
    order = SECOND
    family = LAGRANGE
  [../]
  [./vel_y]
    order = SECOND
    family = LAGRANGE
  [../]
  [./p]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./mass]
    type = INSMass
    variable = p
    u = vel_x
    v = vel_y
  [../]
  [./x_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_x
    u = vel_x
    v = vel_y
    p = p
    component = 0
    integrate_p_by_parts = false
  [../]
  [./y_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_y
    u = vel_x
    v = vel_y
    p = p
    component = 1
    integrate_p_by_parts = false
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'rho mu'
    prop_values = '0.5 1.5'
  [../]
[]

[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
[]

[ICs]
  [./p]
    type = RandomIC
    variable = p
    min = 0.5
    max = 1.5
  [../]
  [./vel_x]
    type = RandomIC
    variable = vel_x
    min = 0.5
    max = 1.5
  [../]
  [./vel_y]
    type = RandomIC
    variable = vel_y
    min = 0.5
    max = 1.5
  [../]
[]
