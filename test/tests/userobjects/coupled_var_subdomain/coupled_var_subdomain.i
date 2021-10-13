[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmin = -1
    xmax = 1
  []
[]

[Variables]
  [u]
  []
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = '10 * x'
  []
[]

[Kernels]
  [u]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = -1E5
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1E5
  []
[]

[UserObjects]
  [coupled_var_subdomain]
    type = CoupledVarSubdomainModifier
    coupled_var = u
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
