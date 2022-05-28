diff=1e-3

[Modules]
  [NavierStokesFV]
    # General parameters
    compressibility = 'incompressible'
    add_flow_equations = false
    add_scalar_equation = true

    # Variables, defined below
    velocity_variable = 'u v'
    pressure_variable = 'pressure'

    # Numerical scheme
    passive_scalar_advection_interpolation = 'average'

    # Precursor equations
    passive_scalar_names = 'scalar'
    passive_scalar_diffusivity = '${diff}'
    passive_scalar_source = '0.1'

    # Inlet boundary conditions
    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '1 0'
    passive_scalar_inlet_types = 'fixed-value'
    passive_scalar_inlet_function = '1'

    # Outlet boundary conditions
    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = 0
  []
[]

[GlobalParams]
  block = 0
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = -1
    ymax = 1
    nx = 100
    ny = 20
  []
[]

[Variables]
  [scalar]
    type = INSFVScalarFieldVariable
  []
[]

[AuxVariables]
  [ax]
    type = MooseVariableFVReal
  []
  [ay]
    type = MooseVariableFVReal
  []
  [u]
    type = INSFVVelocityVariable
  []
  [v]
    type = INSFVVelocityVariable
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
