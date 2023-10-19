[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 20
  []
[]

[Variables]
  [fe][]
  [fv]
    type = MooseVariableFVReal
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = fe
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = fv
    coeff = 1
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = fe
    value = 0
    boundary = left
   []
   [right]
     type = DirichletBC
     variable = fe
     value = 1
     boundary = right
   []
 []

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = fv
    value = 0
    boundary = left
  []
  [right]
    type = FVFunctorNeumannBC
    variable = fv
    functor = fe
    boundary = right
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
