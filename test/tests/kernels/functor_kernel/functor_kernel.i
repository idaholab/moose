# Heat conduction with fixed temperature on left and convection BC on right.
# Source is temperature-dependent:
#   S(T)    = B   - A   * (T - T_inf)^2 [W]
#   S'''(T) = B/V - A/V * (T - T_inf)^2 [W/m^3]
# Assume volume V = 1 m^3, so
#   S'''(T) = B   - A   * (T - T_inf)^2 [W/m^3]

k = 15.0
htc = 100.0
T_ambient = 300.0
source_coef_A = 0.1
source_coef_B = 1e4

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Variables]
  [T]
  []
[]

[FunctorMaterials]
  [source_mat]
    type = ADParsedFunctorMaterial
    expression = 'B - A * (T - T_inf)^2'
    functor_symbols = 'T T_inf A B'
    functor_names = 'T ${T_ambient} ${source_coef_A} ${source_coef_B}'
    property_name = 'source_term'
  []
  [heat_flux_mat]
    type = ADParsedFunctorMaterial
    expression = 'htc * (T - T_inf)'
    functor_symbols = 'T T_inf htc'
    functor_names = 'T ${T_ambient} ${htc}'
    property_name = 'heat_flux'
  []
[]

[Kernels]
  [diff]
    type = FunctionDiffusion
    variable = T
    function = ${k}
  []
  [source]
    type = FunctorKernel
    variable = T
    functor = source_term
    functor_on_rhs = true
  []
[]

[BCs]
  [left_bc]
    type = DirichletBC
    variable = T
    boundary = left
    value = ${T_ambient}
  []
  [right_bc]
    type = FunctorNeumannBC
    variable = T
    boundary = right
    functor = heat_flux
    flux_is_inward = false
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_max_its = 10
  l_max_its = 10
  l_tol = 1e-3
[]

[Outputs]
  exodus = true
[]
