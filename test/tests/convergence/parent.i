# Heat conduction with fixed temperature on left and convection BC on right:
#
#   d/dx(-k dT/dx) = S'''(T)    (0,1)X(0,1)
#   T = T_inf                    x = 0
#   -k dT/dx = htc (T - T_inf)   x = 1
#
# Source is temperature-dependent and is calculated in the child app:
#   S(T) = B - A * (T - T_inf)^2

k = 15.0
htc = 100.0
T_ambient = 300.0

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]

[Variables]
  [T]
  []
[]

[AuxVariables]
  [S_parent]
  []
[]

[FunctorMaterials]
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
    type = CoupledForce
    variable = T
    v = S_parent
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
  multiapp_fixed_point_convergence = fp_conv
[]

[MultiApps]
  [source_app]
    type = FullSolveMultiApp
    positions = '0 0 0'
    input_files = child.i
    execute_on = 'TIMESTEP_END'
  []
[]

[Transfers]
  [T_to_child]
    type = MultiAppCopyTransfer
    to_multi_app = source_app
    source_variable = T
    variable = T_child
    execute_on = 'SAME_AS_MULTIAPP'
  []
  [S_from_child]
    type = MultiAppCopyTransfer
    from_multi_app = source_app
    source_variable = S
    variable = S_parent
    execute_on = 'SAME_AS_MULTIAPP'
  []
[]

[Postprocessors]
  [n_fp_its]
    type = NumFixedPointIterations
    execute_on = 'TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
