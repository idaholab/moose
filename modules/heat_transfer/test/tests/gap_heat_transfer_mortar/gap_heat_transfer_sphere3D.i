sphere_outer_htc = 10 # W/m^2/K
sphere_outer_Tinf = 300 # K

[GlobalParams]
  order = SECOND
  family = LAGRANGE
[]

[Mesh]
  file = sphere3D.e
[]

[Functions]
  [temp]
    type = PiecewiseLinear
    x = '0   1'
    y = '100 200'
  []
[]

[Variables]
  [temp]
    initial_condition = 500
  []
[]

[AuxVariables]
  [gap_conductance]
    order = CONSTANT
    family = MONOMIAL
  []
  [power_density]
    block = 'fuel'
    initial_condition = 50e3
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temp
  []
  [heat_source]
    type = CoupledForce
    variable = temp
    block = 'fuel'
    v = power_density
  []
[]

[AuxKernels]
  [gap_cond]
    type = MaterialRealAux
    property = gap_conductance
    variable = gap_conductance
    boundary = 2
  []
[]

[Materials]
  [heat1]
    type = HeatConductionMaterial
    block = '1 2'
    specific_heat = 1.0
    thermal_conductivity = 34.6
  []
[]

[ThermalContact]
  [thermal_contact]
    type = GapHeatTransfer
    variable = temp
    primary = 3
    secondary = 2
    emissivity_primary = 0
    emissivity_secondary = 0
    gap_conductivity = 5
    gap_geometry_type = SPHERE
    sphere_origin = '0 0 0'
  []
[]

[BCs]
  [RPV_out_BC] # k \nabla T = h (T- T_inf) at RPV outer boundary
    type = ConvectiveFluxFunction # (Robin BC)
    variable = temp
    boundary = '4' # outer RPV
    coefficient = ${sphere_outer_htc}
    T_infinity = ${sphere_outer_Tinf}
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'

  dt = 1
  dtmin = 0.01
  end_time = 1

  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-7

  [Quadrature]
    order = fifth
    side_order = seventh
  []
[]

[Outputs]
  exodus = true
  csv = true
  [Console]
    type = Console
  []
[]

[Postprocessors]
  [temp_left]
    type = SideAverageValue
    boundary = 2
    variable = temp
  []

  [temp_right]
    type = SideAverageValue
    boundary = 3
    variable = temp
  []

  [flux_left]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = 2
    diffusivity = thermal_conductivity
  []
  [flux_right]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = 3
    diffusivity = thermal_conductivity
  []
  [ptot]
    type = ElementIntegralVariablePostprocessor
    variable = power_density
    block = 'fuel'
  []
  [sphere_convective_out]
    type = ConvectiveHeatTransferSideIntegral
    T_solid = temp
    boundary = '4' # outer RVP
    T_fluid = ${sphere_outer_Tinf}
    htc = ${sphere_outer_htc}
  []
  [heat_balance] # should be equal to 0 upon convergence
    type = ParsedPostprocessor
    function = '(sphere_convective_out - ptot) / ptot'
    pp_names = 'sphere_convective_out ptot'
  []
[]

[VectorPostprocessors]
  [NodalTemperature]
    type = NodalValueSampler
    sort_by = id
    boundary = '2 3'
    variable = temp
  []
[]
