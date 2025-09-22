[Mesh/fuel_pin]
  type = FileMeshGenerator
  file = fuel_pin.e
[]

[Physics/HeatConduction/FiniteElement/heat_conduction]
  # Solve heat conduction on fuel and cladding
  block = 'fuel clad'

  # Apply to the variable "T" with an initial condition
  temperature_name = T
  initial_temperature = 310 # [K]

  # Fix the outer boundary temperature to the fluid boundary temperature
  fixed_temperature_boundaries = water_solid_interface
  boundary_temperatures = T_fluid # [K]

  # Insulate the inner boundary (zero heat flux)
  insulated_boundaries = inner

  # Name of the material properties
  thermal_conductivity = k
  specific_heat = cp
  density = rho

  # Numerical parameters
  use_automatic_differentiation = false
[]

# Define a field variable that stores the temperature of
# fluid on the outer boundary (defined in the clad because
# boundary restriction not currently possible)
[AuxVariables/T_fluid]
  initial_condition = 300
  boundary = clad
[]

# Apply a constant heat source to the fuel
[Kernels/heat_source]
  type = BodyForce
  variable = T
  value = 1e8 # [W/m^2 in 2D]
[]

[AuxVariables]
  # We do not have boundary-restricted variables so we define the field on the clad
  [T_fluid]
    block = 'clad'
    initial_condition = 300
  []
  # To store the heat flux computation
  [heat_flux]
    [AuxKernel]
      type = DiffusionFluxAux
      diffusivity = k
      diffusion_variable = T
      component = normal
      boundary = 'inner water_solid_interface'
    []
    # we compute the heat flux on the boundary cell centers rather than
    # nodes because linear-Lagrange T does not have a well defined gradient at nodes
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Materials]
  [fuel]
    type = GenericConstantMaterial
    prop_names = 'k cp rho'
    prop_values = '2 3100 10700' # [W/m*K], [W/K*kg], [kg/m3]
    block = fuel
  []
  [clad]
    type = GenericConstantMaterial
    prop_names = 'k cp rho'
    prop_values = '10 2800 5400' # [W/m*K], [W/K*kg], [kg/m3]
    block = clad
  []
[]

[MultiApps]
  [fluid]
    type = TransientMultiApp
    input_files = step6_fluid.i

    execute_on = TIMESTEP_END
    # We are performing fixed point iterations, this preserves
    # the end of timestep solution as the initial guess for the next solve
    no_restore = true
  []
[]

[Transfers]
  # Dirichlet-Neumann coupling as an example
  # See tutorial on Cardinal website for more techniques,
  # notably renormalizing the heat flux
  [send_heat_flux]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = fluid
    source_variable = 'heat_flux'
    variable = 'flux_from_solid'
    to_boundaries = 'water_solid_interface'
  []
  [receive_Tfluid]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = fluid
    source_variable = 'T_fluid'
    variable = 'T_fluid'
    to_boundaries = 'water_solid_interface'
  []
[]

[Executioner]
  type = Transient
  num_steps = 100
  dt = 1
  # Without fixed point iterations, this first order implicit-explicit scheme is stable
  # until dt=0.008s
  # With fixed point iterations, we obtain the first order implicit Euler scheme, stable
  # for this problem until very high dt!
  # Note that the multiapps has it own dt, and by default min(all dts) is used

  # Nonlinear solver parameters
  automatic_scaling = true
  nl_abs_tol = 1e-10
  line_search = none
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  # Perform fixed point iterations with the multiapp; with this
  # we obtain the first order implicit Euler scheme. Without this,
  # we get loose coupling
  fixed_point_max_its = 30
  fixed_point_rel_tol = 1e-3
  fixed_point_abs_tol = 1e-5

  # The CHT problem needs relaxation, or very small time steps
  relaxation_factor = 0.3
  transformed_variables = 'T'
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  # at steady state, we expect the coupling to have converged and few iterations
  [num_coupling_steps]
    type = NumFixedPointIterations
  []
  [max_T]
    type = NodalExtremeValue
    variable = T
  []
  # sanity check on the integral of the heat flux = integral of heat source at convergence
  # with CGFE: we don't get exact conservation. Refine the mesh, the polynomial order, or use DG or FV!
  [heat_flux]
    type = SideIntegralVariablePostprocessor
    variable = heat_flux
    boundary = 'water_solid_interface'
  []
  [heat_source]
    type = FunctionElementIntegral
    function = '1e8'
    block = 'fuel'
  []
[]
