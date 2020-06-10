[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./t_infinity]
  [../]

  [./active]
    initial_condition = 1
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./force]
    type = BodyForce
    variable = u
    value = 1000
  [../]
[]

[AuxKernels]
  [./t_infinity]
    type = ConstantAux
    variable = t_infinity
    value = 500
    execute_on = initial
  [../]

  [./active_right]
    type = ConstantAux
    variable = active
    value = 0
    boundary = right
  [../]
[]

[BCs]
  [./right]
    type = CoupledConvectiveHeatFluxBC
    variable = u
    boundary = 'left right top bottom'
    htc = 10
    T_infinity = t_infinity
    scale_factor = active
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
