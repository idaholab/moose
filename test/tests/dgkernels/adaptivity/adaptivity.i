# This input file is used for two tests:
# 1) Check that DGKernels work with mesh adaptivity
# 2) Error out when DGKernels are used with adaptivity
#    and stateful material prpoerties

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
  parallel_type = 'replicated'
[]

[Variables]
  [./u]
    order = FIRST
    family = MONOMIAL

    [./InitialCondition]
      type = ConstantIC
      value = 1
    [../]
  [../]
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    expression = (x*x*x)-6.0*x
  [../]

  [./bc_fn]
    type = ParsedFunction
    expression = (x*x*x)
  [../]
[]

[Kernels]
  [./diff]
    type = MatDiffusionTest
    variable = u
    prop_name = diffusivity
  [../]
  [./abs]
    type = Reaction
    variable = u
  [../]
  [./forcing]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[DGKernels]
  [./dgdiff]
    type = DGDiffusion
    variable = u
    sigma = 6
    epsilon = -1.0
    diff = diffusivity
  [../]
[]

[BCs]
  active = 'all'

  [./all]
    type = DGMDDBC
    variable = u
    boundary = '1 2 3 4'
    function = bc_fn
    prop_name = diffusivity
    sigma = 6
    epsilon = -1.0
  [../]
[]

[Materials]
  active = 'constant'
  [./stateful]
    type = StatefulTest
    prop_names = 'diffusivity'
    prop_values = '1'
  [../]
  [./constant]
    type = GenericConstantMaterial
    prop_names = 'diffusivity'
    prop_values = '1'
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Adaptivity]
  marker = 'marker'
  steps = 1
  [./Indicators]
    [./error]
      type = GradientJumpIndicator
      variable = u
    [../]
  [../]
  [./Markers]
    [./marker]
      type = ErrorFractionMarker
      coarsen = 0.5
      indicator = error
      refine = 0.5
    [../]
  [../]
[]

[Outputs]
  exodus = true
[]
