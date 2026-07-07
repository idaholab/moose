# Base input file for a two-variable MMS test problem:
#
#  -div(D_u grad(u)) + u^3 - alpha v  = f_u      u(0) = u(1) = 1
#  -div(D_v grad(v)) + v^3 - beta u^2 = f_v      v(0) = v(1) = 1/2
#
# The manufactured solutions are:
#
#  u(x) = 1 + sin(pi x)
#  y(x) = 1/2 * (1 + sin(pi x))

Du = 1.0
Dv = 0.1
alpha = 5.0
beta = 2.0

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 100
    xmin = 0.0
    xmax = 1.0
  []
[]

[Variables]
  [u]
  []
  [v]
  []
[]

# Manufactured solutions
[Functions]
  [u_fn]
    type = ParsedFunction
    expression = '1 + sin(pi*x)'
  []
  [v_fn]
    type = ParsedFunction
    expression = '0.5 + 0.5 * sin(pi*x)'
  []
[]

[FunctorMaterials]
  [u_src_fmat]
    type = ADParsedFunctorMaterial
    expression = '${Du} * pi^2 * sin(pi*x) + u^3 - ${alpha} * v'
    functor_symbols = 'u v'
    functor_names = 'u_fn v_fn'
    property_name = u_src
  []
  [v_src_fmat]
    type = ADParsedFunctorMaterial
    expression = '0.5 * ${Dv} * pi^2 * sin(pi*x) + v^3 - ${beta} * u^2'
    functor_symbols = 'u v'
    functor_names = 'u_fn v_fn'
    property_name = v_src
  []
  [u_reaction_src_fmat]
    type = ADParsedFunctorMaterial
    expression = 'u^3 - ${alpha}*v - fu'
    functor_symbols = 'u v fu'
    functor_names = 'u v u_src'
    property_name = u_reaction_src_prop
  []
  [v_reaction_src_fmat]
    type = ADParsedFunctorMaterial
    expression = 'v^3 - ${beta}*u^2 - fv'
    functor_symbols = 'u v fv'
    functor_names = 'u v v_src'
    property_name = v_reaction_src_prop
  []
[]

[Kernels]
  [u_diff]
    type = Diffusion
    variable = u
  []
  [u_reaction_src]
    type = FunctorKernel
    variable = u
    functor = u_reaction_src_prop
    functor_on_rhs = false
  []

  [v_diff]
    type = Diffusion
    variable = v
  []
  [v_reaction]
    type = FunctorKernel
    variable = v
    functor = v_reaction_src_prop
    functor_on_rhs = false
  []
[]

[BCs]
  [u_bc]
    type = DirichletBC
    variable = u
    boundary = 'left right'
    value = 1
  []
  [v_bc]
    type = DirichletBC
    variable = v
    boundary = 'left right'
    value = 0.5
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [exodus]
    type = Exodus
    show = 'u v'
    execute_on = 'FINAL'
  []
[]
