[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 10
    ny = 10
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[AuxVariables]
  [v]
  []
[]

[ICs]
  [v_ic]
    type = FunctionIC
    variable = v
    function = v_fn
  []
[]

[Functions]
  [v_fn]
    type = ParsedFunction
    expression = 'y - 0.5'
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
    type = FunctorNeumannBC
    variable = u
    boundary = right
    functor = v
    coefficient = 0.5
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
