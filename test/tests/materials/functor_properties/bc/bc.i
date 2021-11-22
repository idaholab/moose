[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [v][]
[]

[Kernels]
  [diff]
    type = FunctorMatDiffusion
    variable = v
    diffusivity = 1
  []
  [source]
    type = BodyForce
    variable = v
    function = 'x + y'
  []
[]

[BCs]
  [bounds]
    type = MatPropBC
    variable = v
    boundary = 'left right top bottom'
    mat_prop = 'prop'
  []
[]

[Materials]
  [functor]
    type = ADGenericFunctorMaterial
    prop_names = 'prop'
    prop_values = 'v'
    execute_on = 'LINEAR NONLINEAR'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
