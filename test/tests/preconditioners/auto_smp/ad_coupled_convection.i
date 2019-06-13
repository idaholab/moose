[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u][]
  [v][]
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = u
  []
  [convection]
    type = ADCoupledConvection
    variable = u
    velocity_vector = v
    scale = 100
  []
  [diff_v]
    type = ADDiffusion
    variable = v
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  []
[]

[Preconditioning/smp]
  # this block is part of what is being tested, see "tests" file
  type = SMP
  full = true
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  auto_preconditioning = false # this is part of what is being tested, see "tests" file
  nl_max_its = 8               # needed to get non-preconditioned version to fail
[]

[Outputs]
  exodus = true
[]
