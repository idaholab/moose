mu = 1
rho = 1
k = .01
cp = 1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 32
    ny = 32
  []
[]

[Modules]
  [NavierStokesFV]
    simulation_type = 'steady-state'
    compressibility = 'incompressible'
    porous_medium_treatment = false
    add_energy_equation = true

    density = 'rho'
    dynamic_viscosity = 'mu'
    thermal_conductivity = 'k'
    specific_heat = 'cp'

    inlet_boundaries = 'top'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = 'lid_function 0'
    energy_inlet_types = 'fixed-temperature'
    energy_inlet_function = '0'

    wall_boundaries = 'left right bottom'
    momentum_wall_types = 'noslip noslip noslip'
    energy_wall_types = 'heatflux heatflux fixed-temperature'
    energy_wall_function = '0 0 1'

    pin_pressure = true
    pinned_pressure_type = average
    pinned_pressure_value = 0
  []
[]

[AuxVariables]
  [U]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[AuxKernels]
  [mag]
    type = VectorMagnitudeAux
    variable = U
    x = vel_x
    y = vel_y
  []
[]

[Materials]
  [functor_constants]
    type = ADGenericFunctorMaterial
    prop_names = 'cp k rho mu'
    prop_values = '${cp} ${k} ${rho} ${mu}'
  []
[]

[Functions]
  [lid_function]
    type = ParsedFunction
    value = '4*x*(1-x)'
  []
[]


[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      300                lu           NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
