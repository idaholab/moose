n=5

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = ${n}
    ny = ${n}
    nz = ${n}
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff_u]
    type = FunctorMatDiffusion
    variable = u
    diffusivity = 'slow_prop'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 0
  []
[]

[Materials]
  [slow_prop]
    type = ReallyExpensiveFunctorMaterial
    execute_on = 'always'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
