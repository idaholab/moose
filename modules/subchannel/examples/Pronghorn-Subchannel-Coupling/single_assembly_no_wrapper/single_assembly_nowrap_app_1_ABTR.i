# Flow in a single ABTR assembly with no wrapper
# modeled with porous medium in Pronghorn
# Single porosity region (Elia approach 1)

# Units are SI

# geometry parameters
pin_diameter = 8e-3
pin_pitch = 9.04e-3
flat_to_flat = 14.598e-2
wire_diameter = 1.03e-3
wire_pitch = 203.2e-3

# physics parameters
inlet_temperaure = 500
outlet_pressure = 0 # gauge pressure

# fluid properties
rho = 1000
mu = 2.27e-4
cp = 1500
k = 60

# heat source
heat_source = 1e8

[Debug]
  show_material_props = true
[]

[Mesh]
  [fmesh]
    type = FileMeshGenerator
    file = 'single_region.e'
  []

  [add_top]
    type = SideSetsFromNormalsGenerator
    normals = '0 0 1'
    input = fmesh
    new_boundary = top
  []

  [add_bottom]
    type = SideSetsFromNormalsGenerator
    normals = '0 0 -1'
    input = add_top
    new_boundary = bottom
  []
[]

[UserObjects]
  [hex]
    type = HexagonalLattice
    bundle_pitch = ${flat_to_flat}
    pin_pitch = ${pin_pitch}
    pin_diameter = ${pin_diameter}
    wire_diameter = ${wire_diameter}
    wire_pitch = ${wire_pitch}
    n_rings = 9
  []
[]

[Functions]
  [inlet_vel_fn]
    type = PiecewiseLinear
    x = '0     1'
    y = '1e-15 1'
  []
[]

[Modules]
  [NavierStokesFV]
    # general input parameters
    compressibility = 'incompressible'
    add_energy_equation = true

    # porous media parameters
    porous_medium_treatment = true
    porosity = porosity_functor
    friction_types = 'darcy forchheimer'
    friction_coeffs = 'Darcy_coefficient Forchheimer_coefficient'
    #porosity_smoothing_layers = 2

    # heating source
    external_heat_source = heat_source

    # material properties
    density = 'rho'
    dynamic_viscosity = 'mu'
    thermal_conductivity = 'k'
    specific_heat = 'cp'

    # initial conditions
    initial_velocity = '1e-15 1e-15 1e-15'
    initial_temperature = '${inlet_temperaure}'
    initial_pressure = '${outlet_pressure}'

    # boundary conditions
    inlet_boundaries = 'bottom'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '0 0 inlet_vel_fn'
    energy_inlet_types = 'fixed-temperature'
    energy_inlet_function = '${inlet_temperaure}'

    wall_boundaries = 'wall'
    momentum_wall_types = 'noslip'
    energy_wall_types = 'heatflux'
    energy_wall_function = '0'

    outlet_boundaries = 'top'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '${outlet_pressure}'
  []
[]

[AuxVariables]
  [porosity]
    type = MooseVariableFVReal
  []

  [heat_source]
    type = MooseVariableFVReal
    initial_condition = ${heat_source}
  []
[]

[AuxKernels]
  [porosity_aux]
    type = HexLatticePorosityAux
    variable = porosity
    hex_lattice = hex
    num_regions = ONE
    execute_on = 'initial'
  []
[]

[Materials]
  [functor_constants]
    type = ADGenericFunctorMaterial
    prop_names =  'cp    k    rho    mu'
    prop_values = '${cp} ${k} ${rho} ${mu}'
  []

  [porosity_functor_mat]
    type = ADGenericFunctorMaterial
    prop_names = porosity_functor
    prop_values = porosity
  []

  [drag]
    type = FunctorRehmeDragCoefficients
    hex_lattice = hex
    multipliers = '100 100 1'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      300                lu           NONZERO'
  dt = 0.1
  nl_abs_tol = 1e-4
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
