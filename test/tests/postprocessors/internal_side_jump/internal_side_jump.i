[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
  [./box]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '0.5 0.5 0'
    block_id = 1
  [../]
[]

[Variables]
  [./u]
    family = L2_LAGRANGE
    order = FIRST
  [../]
[]

[ICs]
  [./ic0]
    type = ConstantIC
    variable = u
    block = 0
    value = 4
  [../]
  [./ic1]
    type = ConstantIC
    variable = u
    block = 1
    value = 6
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[DGKernels]
  [./dgdiff]
    type = DGDiffusion
    variable = u
    sigma = 4
    epsilon = 1
  [../]
[]

[BCs]
  [./all]
    type = VacuumBC
    variable = u
    boundary = '0 1 2 3'
  [../]
[]

[Postprocessors]
  [./L2_norm]
    type = ElementL2Norm
    variable = u
  [../]
  [./jump]
    type = InternalSideJump
    variable = u
    execute_on = 'initial timestep_end'
  [../]
  [./jumpold]
    type = InternalSideJump
    variable = u
    implicit = false
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 3
  nl_abs_tol = 1e-12
[]

[Outputs]
  csv = true
[]
