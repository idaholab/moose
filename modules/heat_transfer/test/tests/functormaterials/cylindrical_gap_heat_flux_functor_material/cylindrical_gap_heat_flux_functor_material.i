[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
[]

[AuxVariables]
  [q_cond]
    family = MONOMIAL
    order = CONSTANT
  []
  [q_rad]
    family = MONOMIAL
    order = CONSTANT
  []
  [q_total]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [q_cond_kern]
    type = FunctorElementalAux
    variable = q_cond
    functor = conduction_heat_flux
    execute_on = 'INITIAL'
  []
  [q_rad_kern]
    type = FunctorElementalAux
    variable = q_rad
    functor = radiation_heat_flux
    execute_on = 'INITIAL'
  []
  [q_total_kern]
    type = FunctorElementalAux
    variable = q_total
    functor = total_heat_flux
    execute_on = 'INITIAL'
  []
[]

[Functions]
  [r_outer_fn]
    type = ParsedFunction
    # vary gap distance from 1 um to 1 mm in (0,1)
    expression = '1.0 + 10^(-6 + 3*z)'
  []
  [T_inner_fn]
    type = ParsedFunction
    expression = '300 + 1000 * x'
  []
  [T_outer_fn]
    type = ParsedFunction
    expression = '300 + 1000 * y'
  []
[]

[Materials]
  [heat_flux_fmat]
    type = CylindricalGapHeatFluxFunctorMaterial
    r_inner = 1.0
    r_outer = r_outer_fn
    emissivity_inner = 0.25
    emissivity_outer = 0.75
    k_gap = 0.15
    T_inner = T_inner_fn
    T_outer = T_outer_fn
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
