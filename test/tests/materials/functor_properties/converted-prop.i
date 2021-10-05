[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 2
  []
[]

[Variables]
  [u][]
[]

[Kernels]
  [diff_u]
    type = ADFunctorMatDiffusion
    variable = u
    diffusivity = 'fe_prop'
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
  [prop_used_by_kernel]
    type = IMakeMyOwnFunctorProps
    retrieved_prop_name = 'upcast_prop'
    fe_var = 'u'
    fe_prop_name = 'fe_prop'
  []
  [prop_used_by_mat]
    type = IMakeMyOwnFunctorProps
    fe_var = 'u'
    fe_prop_name = 'upcast_prop'
  []
[]


[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
