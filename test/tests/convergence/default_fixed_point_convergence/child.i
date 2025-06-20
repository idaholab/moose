# Solves the nonlinear equation
#   S(T) = B - A * (T - T_inf)^2
# on each node.

T_ambient = 300.0
source_coef_A = 0.1
source_coef_B = 1e4

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [S]
  []
[]

[AuxVariables]
  [T_child]
  []
[]

[FunctorMaterials]
  [equation_mat]
    type = ADParsedFunctorMaterial
    expression = 'B - A * (T - T_inf)^2 - S'
    functor_symbols = 'T T_inf A B S'
    functor_names = 'T_child ${T_ambient} ${source_coef_A} ${source_coef_B} S'
    property_name = 'equation'
  []
[]

[Kernels]
  [equation_kernel]
    type = FunctorKernel
    variable = S
    functor = equation
    functor_on_rhs = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_max_its = 10
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  l_max_its = 10
  l_tol = 1e-3
[]
