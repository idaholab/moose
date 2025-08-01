# Solves the nonlinear equation
#   S(T) = B - A * (T - T_inf)^2
# on each node.

T_ambient = 300.0
source_coef_A = 0.1
source_coef_B = 1e4

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
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
[]
