[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = -1.1
  ymin = -1.1
  xmax = 1.1
  ymax = 1.1
[]

[Variables]
  [./v]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = v
  [../]
  [./source]
    type = BodyForce
    variable = v
  [../]
  [./advection]
    type = EFieldAdvection
    variable = v
    charge = 'positive'
    mobility = 1
  [../]
[]

[BCs]
  [left]
    type = DirichletBC
    variable = v
    value = 0
    boundary = left
  []
  [right]
    type = DirichletBC
    variable = v
    value = 1
    boundary = right
  []
[]

[Preconditioning]
  [./pre]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'asm'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
[]

[Outputs]
  exodus = true
[]
