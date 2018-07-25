# Test of advection with full upwinding
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 3
  ny = 2
  nz = 1
[]

[Variables]
  [./u]
  [../]
[]

[ICs]
  [./u]
    type = RandomIC
    variable = u
  [../]
[]

[Kernels]
  [./advection]
    type = ConservativeAdvection
    variable = u
    upwinding_type = full
    velocity = '2 -1.1 1.23'
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-snes_type'
  petsc_options_value = 'test'
  dt = 2
  end_time = 2
[]
