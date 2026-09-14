# Differentially heated cavity with conducting slabs on the left and right.
#
# Domain layout:
#
#   T = 880 K              fluid cavity               T = 870 K
#       |       left slab | 0 < x < 1 | right slab       |
#       |   -d < x < 0    |           | 1 < x < 1+d      |
#
# The prescribed temperatures are applied only to the OUTER slab faces.
# The left/right cavity surfaces use linear-FV CHT. Surface-to-surface
# radiation is passed into CHTHandler through surface_radiation_object_name.
# The top/bottom cavity walls are externally adiabatic, so their conductive
# heat flux is balanced by LinearFVGrayLambertBC.

################################################################################
# MATERIAL PROPERTIES AND GEOMETRY
################################################################################

rho = 3279.0
T_0 = 875.0
T_hot = 880.0
T_cold = 870.0
mu = 1.0
k_fluid = 38.0
k_solid = 38.0
cp = 640.0
alpha = 3.26e-5

eps = 1.0

slab_thickness = 0.10
n_slab_x = 20
n_fluid_x = 200
n_y = 120

rad_left = 'left_0 left_1 left_2 left_3 left_4'
rad_right = 'right_0 right_1 right_2 right_3 right_4'
rad_top = 'top_0 top_1 top_2 top_3 top_4'
rad_bottom = 'bottom_0 bottom_1 bottom_2 bottom_3 bottom_4'

rad_cht = '${rad_left} ${rad_right}'
rad_noncht = '${rad_top} ${rad_bottom}'
rad_all = '${rad_left} ${rad_right} ${rad_top} ${rad_bottom}'
walls = ${rad_all}


[GlobalParams]
  rhie_chow_user_object = ins_rhie_chow_interpolator
  advected_interp_method = upwind
  u = vel_x
  v = vel_y
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system solid_energy_system'
  previous_nl_solution_required = true
[]

################################################################################
# GEOMETRY
################################################################################

[Mesh]
  type = MeshGeneratorMesh
  parallel_type = replicated

  # A single generator gives a conforming mesh across both CHT interfaces.
  [base]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${slab_thickness} 1.0 ${slab_thickness}'
    ix = '${n_slab_x} ${n_fluid_x} ${n_slab_x}'
    dy = '1.0'
    iy = '${n_y}'
    subdomain_id = '1 0 2'
  []

  # Keep the fluid cavity at 0 <= x <= 1.
  [shift]
    type = TransformGenerator
    input = base
    transform = TRANSLATE
    vector_value = '${fparse -slab_thickness} 0 0'
  []

  [rename_blocks]
    type = RenameBlockGenerator
    input = shift
    old_block = '0 1 2'
    new_block = 'fluid left_solid right_solid'
  []

  # Reserve left/right/top/bottom for the four surfaces of the cavity.
  [rename_exterior]
    type = RenameBoundaryGenerator
    input = rename_blocks
    old_boundary = 'left right bottom top'
    new_boundary = 'left_outer right_outer whole_bottom whole_top'
  []

  # Put the internal sidesets on fluid elements so gray_lambert can sample
  # T_fluid on every surface in its boundary list.
  [left_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = rename_exterior
    primary_block = fluid
    paired_block = left_solid
    new_boundary = left
  []

  [right_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = left_interface
    primary_block = fluid
    paired_block = right_solid
    new_boundary = right
  []

  # Re-create separate exterior horizontal sidesets for each block.
  [cavity_horizontal]
    type = SideSetsFromNormalsGenerator
    input = right_interface
    normals = '0 -1 0
               0  1 0'
    fixed_normal = true
    included_subdomains = fluid
    new_boundary = 'bottom top'
  []

  [left_slab_horizontal]
    type = SideSetsFromNormalsGenerator
    input = cavity_horizontal
    normals = '0 -1 0
               0  1 0'
    fixed_normal = true
    included_subdomains = left_solid
    new_boundary = 'left_solid_bottom left_solid_top'
  []

  [right_slab_horizontal]
    type = SideSetsFromNormalsGenerator
    input = left_slab_horizontal
    normals = '0 -1 0
               0  1 0'
    fixed_normal = true
    included_subdomains = right_solid
    new_boundary = 'right_solid_bottom right_solid_top'
  []

  # Patch every radiating surface. This allows the wall temperature and
  # radiative heat flux to vary along each side of the cavity.
  [patch_left]
    type = PatchSidesetGenerator
    input = right_slab_horizontal
    boundary = left
    n_patches = 5
    partitioner = centroid
    centroid_partitioner_direction = y
  []

  [patch_right]
    type = PatchSidesetGenerator
    input = patch_left
    boundary = right
    n_patches = 5
    partitioner = centroid
    centroid_partitioner_direction = y
  []

  [patch_top]
    type = PatchSidesetGenerator
    input = patch_right
    boundary = top
    n_patches = 5
    partitioner = centroid
    centroid_partitioner_direction = x
  []

  [patch_bottom]
    type = PatchSidesetGenerator
    input = patch_top
    boundary = bottom
    n_patches = 5
    partitioner = centroid
    centroid_partitioner_direction = x
  []

  # PatchSidesetGenerator retains its parent sideset. Remove the parents so
  # each face has only its patch boundary for radiation and CHT bookkeeping.
  [delete_unpatched_parents]
    type = BoundaryDeletionGenerator
    input = patch_bottom
    boundary_names = 'left right top bottom whole_top whole_bottom'
  []

  final_generator = delete_unpatched_parents
[]

################################################################################
# USER OBJECTS: RHIE-CHOW AND SURFACE-TO-SURFACE RADIATION
################################################################################

[UserObjects]
  [ins_rhie_chow_interpolator]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure
    rho = ${rho}
    p_diffusion_kernel = p_diffusion
    block = fluid
  []

  # The enclosure is convex and unobstructed. This deterministic view-factor
  # object also avoids ray-orientation ambiguity on the internal CHT sidesets.
  [view_factor]
    type = UnobstructedPlanarViewFactor
    boundary = ${rad_all}
    normalize_view_factor = true
    execute_on = INITIAL
  []

  [gray_lambert]
    type = ViewFactorObjectSurfaceRadiation
    boundary = ${rad_all}

    # There are no fixed-temperature or radiatively adiabatic surfaces here.
    # All 20 surfaces obtain their current temperature from T_fluid.
    emissivity = '${eps} ${eps} ${eps} ${eps} ${eps}
                  ${eps} ${eps} ${eps} ${eps} ${eps}
                  ${eps} ${eps} ${eps} ${eps} ${eps}
                  ${eps} ${eps} ${eps} ${eps} ${eps}'
    temperature = T_fluid
    view_factor_object_name = view_factor

    # Refreshes radiation during the SIMPLE/CHT Picard loop. The handler uses
    # the most recently assembled radiation state (one fluid-energy solve lag).
    execute_on = 'NONLINEAR TIMESTEP_END'
  []
[]

################################################################################
# VARIABLES
################################################################################

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    block = fluid
  []

  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    block = fluid
  []

  [pressure]
    type = MooseLinearVariableFVReal
    initial_condition = 0
    solver_sys = pressure_system
    block = fluid
  []

  [T_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = ${T_0}
    block = fluid
  []

  # A single solid system/variable is used on the two disconnected slabs,
  # as required by the current linear-FV CHTHandler.
  [T_solid]
    type = MooseLinearVariableFVReal
    solver_sys = solid_energy_system
    initial_condition = ${T_0}
    block = 'left_solid right_solid'
  []
[]

[FVInterpolationMethods]
  [upwind]
    type = FVAdvectedUpwind
  []
[]

################################################################################
# LINEAR-FV EQUATIONS
################################################################################

[LinearFVKernels]
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method_name = upwind
    mu = ${mu}
    momentum_component = x
    use_nonorthogonal_correction = false
  []

  [u_pressure]
    type = LinearFVMomentumPressure
    variable = vel_x
    pressure = pressure
    momentum_component = x
  []

  [u_buoyancy]
    type = LinearFVMomentumBoussinesq
    variable = vel_x
    T_fluid = T_fluid
    gravity = '0 -9.81 0'
    rho = ${rho}
    ref_temperature = ${T_0}
    alpha_name = alpha_b
    momentum_component = x
  []

  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    advected_interp_method_name = upwind
    mu = ${mu}
    momentum_component = y
    use_nonorthogonal_correction = false
  []

  [v_pressure]
    type = LinearFVMomentumPressure
    variable = vel_y
    pressure = pressure
    momentum_component = y
  []

  [v_buoyancy]
    type = LinearFVMomentumBoussinesq
    variable = vel_y
    T_fluid = T_fluid
    gravity = '0 -9.81 0'
    rho = ${rho}
    ref_temperature = ${T_0}
    alpha_name = alpha_b
    momentum_component = y
  []

  [p_diffusion]
    type = LinearFVPressureCorrectionDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = false
  []

  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = false
  []

  [fluid_heat_advection]
    type = LinearFVEnergyAdvection
    variable = T_fluid
    advected_quantity = temperature
    cp = ${cp}
  []

  [fluid_conduction]
    type = LinearFVDiffusion
    variable = T_fluid
    diffusion_coeff = ${k_fluid}
    use_nonorthogonal_correction = false
  []

  # CHTHandler expects exactly one solid conduction kernel in the solid
  # energy system, so this one kernel covers both slabs.
  [solid_conduction]
    type = LinearFVDiffusion
    variable = T_solid
    diffusion_coeff = ${k_solid}
    use_nonorthogonal_correction = false
  []
[]

################################################################################
# LINEAR-FV BOUNDARY CONDITIONS
################################################################################

[LinearFVBCs]
  [no_slip_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_x
    boundary = ${walls}
    functor = 0
  []

  [no_slip_v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_y
    boundary = ${walls}
    functor = 0
  []

  [pressure_walls]
    type = LinearFVPressureFluxBC
    boundary = ${walls}
    variable = pressure
    HbyA_flux = HbyA
    Ainv = Ainv
    u = vel_x
    v = vel_y
    rho = ${rho}
  []

  # Prescribe temperature only on the exterior faces of the slabs.
  [left_outer_temperature]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_solid
    boundary = left_outer
    functor = ${T_hot}
  []

  [right_outer_temperature]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_solid
    boundary = right_outer
    functor = ${T_cold}
  []

  [insulated_slab_horizontal_faces]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    variable = T_solid
    boundary = 'left_solid_bottom left_solid_top right_solid_bottom right_solid_top'
    functor = 0
  []

  # Radiation is imposed directly only on the non-CHT top/bottom walls.
  # Do not add these boundaries to adiabatic_boundary in gray_lambert:
  # this BC makes their conductive flux balance their radiative flux.
  [radiation_top_bottom]
    type = LinearFVGrayLambertBC
    variable = T_fluid
    temperature_radiation = T_fluid
    coeff_diffusion = ${k_fluid}
    surface_radiation_object_name = gray_lambert
    boundary = ${rad_noncht}
  []

  # Neumann-Dirichlet CHT. Each patch needs its own pair because CHTHandler
  # creates boundary-specific temperature and heat-flux functors.

  # ---- left slab / fluid interface -----------------------------------------
  [fluid_left_0]
    type = LinearFVDirichletCHTBC
    variable = T_fluid
    boundary = left_0
    functor = interface_temperature_solid_left_0
  []
  [solid_left_0]
    type = LinearFVRobinCHTBC
    variable = T_solid
    boundary = left_0
    h = 0
    thermal_conductivity = ${k_solid}
    incoming_flux = heat_flux_to_solid_left_0
    surface_temperature = interface_temperature_fluid_left_0
  []

  [fluid_left_1]
    type = LinearFVDirichletCHTBC
    variable = T_fluid
    boundary = left_1
    functor = interface_temperature_solid_left_1
  []
  [solid_left_1]
    type = LinearFVRobinCHTBC
    variable = T_solid
    boundary = left_1
    h = 0
    thermal_conductivity = ${k_solid}
    incoming_flux = heat_flux_to_solid_left_1
    surface_temperature = interface_temperature_fluid_left_1
  []

  [fluid_left_2]
    type = LinearFVDirichletCHTBC
    variable = T_fluid
    boundary = left_2
    functor = interface_temperature_solid_left_2
  []
  [solid_left_2]
    type = LinearFVRobinCHTBC
    variable = T_solid
    boundary = left_2
    h = 0
    thermal_conductivity = ${k_solid}
    incoming_flux = heat_flux_to_solid_left_2
    surface_temperature = interface_temperature_fluid_left_2
  []

  [fluid_left_3]
    type = LinearFVDirichletCHTBC
    variable = T_fluid
    boundary = left_3
    functor = interface_temperature_solid_left_3
  []
  [solid_left_3]
    type = LinearFVRobinCHTBC
    variable = T_solid
    boundary = left_3
    h = 0
    thermal_conductivity = ${k_solid}
    incoming_flux = heat_flux_to_solid_left_3
    surface_temperature = interface_temperature_fluid_left_3
  []

  [fluid_left_4]
    type = LinearFVDirichletCHTBC
    variable = T_fluid
    boundary = left_4
    functor = interface_temperature_solid_left_4
  []
  [solid_left_4]
    type = LinearFVRobinCHTBC
    variable = T_solid
    boundary = left_4
    h = 0
    thermal_conductivity = ${k_solid}
    incoming_flux = heat_flux_to_solid_left_4
    surface_temperature = interface_temperature_fluid_left_4
  []

  # ---- right slab / fluid interface ----------------------------------------
  [fluid_right_0]
    type = LinearFVDirichletCHTBC
    variable = T_fluid
    boundary = right_0
    functor = interface_temperature_solid_right_0
  []
  [solid_right_0]
    type = LinearFVRobinCHTBC
    variable = T_solid
    boundary = right_0
    h = 0
    thermal_conductivity = ${k_solid}
    incoming_flux = heat_flux_to_solid_right_0
    surface_temperature = interface_temperature_fluid_right_0
  []

  [fluid_right_1]
    type = LinearFVDirichletCHTBC
    variable = T_fluid
    boundary = right_1
    functor = interface_temperature_solid_right_1
  []
  [solid_right_1]
    type = LinearFVRobinCHTBC
    variable = T_solid
    boundary = right_1
    h = 0
    thermal_conductivity = ${k_solid}
    incoming_flux = heat_flux_to_solid_right_1
    surface_temperature = interface_temperature_fluid_right_1
  []

  [fluid_right_2]
    type = LinearFVDirichletCHTBC
    variable = T_fluid
    boundary = right_2
    functor = interface_temperature_solid_right_2
  []
  [solid_right_2]
    type = LinearFVRobinCHTBC
    variable = T_solid
    boundary = right_2
    h = 0
    thermal_conductivity = ${k_solid}
    incoming_flux = heat_flux_to_solid_right_2
    surface_temperature = interface_temperature_fluid_right_2
  []

  [fluid_right_3]
    type = LinearFVDirichletCHTBC
    variable = T_fluid
    boundary = right_3
    functor = interface_temperature_solid_right_3
  []
  [solid_right_3]
    type = LinearFVRobinCHTBC
    variable = T_solid
    boundary = right_3
    h = 0
    thermal_conductivity = ${k_solid}
    incoming_flux = heat_flux_to_solid_right_3
    surface_temperature = interface_temperature_fluid_right_3
  []

  [fluid_right_4]
    type = LinearFVDirichletCHTBC
    variable = T_fluid
    boundary = right_4
    functor = interface_temperature_solid_right_4
  []
  [solid_right_4]
    type = LinearFVRobinCHTBC
    variable = T_solid
    boundary = right_4
    h = 0
    thermal_conductivity = ${k_solid}
    incoming_flux = heat_flux_to_solid_right_4
    surface_temperature = interface_temperature_fluid_right_4
  []
[]

[FunctorMaterials]
  [constant_fluid_functors]
    type = GenericFunctorMaterial
    # block = fluid
    prop_names = 'cp alpha_b'
    prop_values = '${cp} ${alpha}'
  []
[]

################################################################################
# DIAGNOSTICS
################################################################################

[VectorPostprocessors]
  [surface_radiation]
    type = SurfaceRadiationVectorPostprocessor
    surface_radiation_object_name = gray_lambert
    information = 'temperature emissivity radiosity heat_flux_density'
    execute_on = TIMESTEP_END
  []
[]

################################################################################
# EXECUTION / SOLVE
################################################################################

# CHTHandler requires one value per entry in rad_cht (10 entries here).
cht_relaxation = '0.3 0.3 0.3 0.3 0.3 0.3 0.3 0.3 0.3 0.3'

[Executioner]
  type = SIMPLE

  rhie_chow_user_object = ins_rhie_chow_interpolator
  momentum_systems = 'u_system v_system'
  pressure_system = pressure_system
  energy_system = energy_system
  solid_energy_system = solid_energy_system

  momentum_l_abs_tol = 1e-11
  pressure_l_abs_tol = 1e-11
  energy_l_abs_tol = 1e-11
  solid_energy_l_abs_tol = 1e-11
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  solid_energy_l_tol = 0

  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  energy_equation_relaxation = 0.9

  num_iterations = 5000
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  energy_absolute_tolerance = 1e-8
  solid_energy_absolute_tolerance = 1e-8

  # Activate CHT on every patched slab/cavity interface.
  cht_interfaces = ${rad_cht}
  cht_solid_flux_relaxation = ${cht_relaxation}
  cht_fluid_flux_relaxation = ${cht_relaxation}
  cht_solid_temperature_relaxation = ${cht_relaxation}
  cht_fluid_temperature_relaxation = ${cht_relaxation}
  cht_heat_flux_tolerance = 1e-4
  max_cht_fpi = 10
  surface_radiation_object_name = gray_lambert

  print_fields = false
  momentum_l_max_its = 300

  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '0.5 0.5 0.0'

  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'

  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'

  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'

  solid_energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  solid_energy_petsc_options_value = 'hypre boomeramg'

  continue_on_max_its = true

  # Improves the numerical integration used by UnobstructedPlanarViewFactor.
  [Quadrature]
    order = SECOND
  []
[]

################################################################################
# OUTPUTS
################################################################################

[Outputs]
  exodus = true
  csv = true
  execute_on = TIMESTEP_END
[]
