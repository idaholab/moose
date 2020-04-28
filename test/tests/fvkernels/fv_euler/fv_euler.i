[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 20
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [u]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[FVKernels]
  # A "friction" term
  [rxn]
    type = FVReaction
    variable = u
  []
  [adv]
    type = FVMatAdvection
    variable = u
    vel = 'velocity'
  []
  [diff]
    type = FVDiffusion
    variable = u
    coeff = coeff
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = u
    value = 1
    boundary = 'left'
  []
[]

[Materials]
  [velocity]
    type = ADCoupledVelocityMaterial
    vel_x = u
  []
  [diff]
    type = ADGenericConstantMaterial
    prop_names = 'coeff'
    prop_values = '1'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]
