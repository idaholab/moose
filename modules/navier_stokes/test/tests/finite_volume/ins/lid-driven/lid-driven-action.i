mu = .01
rho = 1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = .1
    ymin = 0
    ymax = .1
    nx = 20
    ny = 20
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'

    density = ${rho}
    dynamic_viscosity = ${mu}

    initial_pressure = 0.0

    inlet_boundaries = 'top'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '1 0'

    wall_boundaries = 'left right bottom'
    momentum_wall_types = 'noslip noslip noslip'

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

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
