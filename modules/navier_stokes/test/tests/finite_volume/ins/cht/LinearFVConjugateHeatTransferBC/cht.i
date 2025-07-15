### benchmark sources:
### https://doi.org/10.1016/j.compfluid.2018.06.016
### https://doi.org/10.1016/0017-9310(74)90087-8

b = 0.01 # plate thickness
l = 0.2  # plate length

nxi  = 10 # nx in the inlet/entrance region
nyf  = 25 # ny in fluid
nxf  = 20 # nx in the main fluid region
nys  = 10 # ny in the fluid domain

fx1_bias = 1.15 # bdry layer bias - fluid
fx2_bias = 0.85 # bdry layer bias - solid
fy_bias  = 1.15 # bdry layer bias - fluid
sy_bias  = 0.85 # bdry layer bias - solid

# TODO: add bias in x for fluid entrance/cht internal boundary if needed

k_s = 0.2876 #${fparse 287.0 * l}
rho = 0.3525
mu  = 3.95e-5
k   = 0.06808
cp  = 1142.6

vin = 12.0
Tin = 1000.0
T_s_bottom = 600.0
P_out = 1.03e5

#h_test = ${fparse k_s/l} # test value
h_s = 20
h_f = 100

advected_interp_method = 'upwind'

[Mesh]
  [fluid_channel]
    type = GeneratedMeshGenerator
    dim = 2
    nx = ${nxf}
    ny = ${nyf}
    xmin = 0
    xmax = ${l}
    ymin = 0
    ymax = ${fparse 2.0*b}
    subdomain_ids  = '1'
    subdomain_name = 'fluid'
    bias_x = ${fx1_bias}
    bias_y = ${fy_bias}
    boundary_name_prefix = 'fluid'
  []
  [solid_base]
    type = GeneratedMeshGenerator
    dim = 2
    nx = ${nxf}
    ny = ${nys}
    xmin = 0
    xmax = ${l}
    ymin = ${fparse -b}
    ymax = 0
    subdomain_ids  = '2'
    subdomain_name = 'solid'
    bias_x = ${fx1_bias}
    bias_y = ${sy_bias}
    boundary_id_offset = 10
    boundary_name_prefix = 'solid'
  []
  [entrance]
    type = GeneratedMeshGenerator
    dim = 2
    nx = ${nxi}
    ny = ${nyf}
    xmin = ${fparse -0.33*l}
    xmax = 0
    ymin = 0
    ymax = ${fparse 2.0*b}
    subdomain_ids  = '0'
    subdomain_name = 'entrance'
    bias_x = ${fx2_bias}
    bias_y = ${fy_bias}
    boundary_id_offset = 20
    boundary_name_prefix = 'ent'
  []
#  [solid]
#    type = SubdomainBoundingBoxGenerator
#    input = fluid_channel
#    bottom_left  = '0 ${fparse -b} 0'
#    top_right = '${l} 0 0'
#    block_id  = 2
#    block_name = 'solid'
#  []
  [smg]
    type = StitchedMeshGenerator
    inputs = 'entrance fluid_channel solid_base'
    stitch_boundaries_pairs = 'ent_right fluid_left;
                              fluid_bottom solid_top'
    show_info = true
    prevent_boundary_ids_overlap = false
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'smg'
    primary_block = 'fluid'
    paired_block  = 'solid'
    new_boundary = interface
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system solid_energy_system'
  previous_nl_solution_required = true
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure
    rho = ${rho}
    p_diffusion_kernel = p_diffusion
    block = '0 1'
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    initial_condition = ${vin}
    solver_sys = u_system
    block = '0 1'
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0.001
    block = '0 1'
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = ${P_out}
    block = '0 1'
  []
  [T_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = ${Tin}
    block = '0 1'
  []
  [T_solid]
    type = MooseLinearVariableFVReal
    solver_sys = solid_energy_system
    initial_condition = ${T_s_bottom}
    block = 2
  []
[]

[LinearFVKernels]
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = true
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = true
  []
  [u_pressure]
    type = LinearFVMomentumPressure
    variable = vel_x
    pressure = pressure
    momentum_component = 'x'
  []
  [v_pressure]
    type = LinearFVMomentumPressure
    variable = vel_y
    pressure = pressure
    momentum_component = 'y'
  []

  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = true
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []

  [h_advection]
    type = LinearFVEnergyAdvection
    variable = T_fluid
    advected_quantity = temperature
    cp = ${cp}
    advected_interp_method = ${advected_interp_method}
    rhie_chow_user_object = 'rc'
  []
  [conduction]
    type = LinearFVDiffusion
    variable = T_fluid
    diffusion_coeff = ${k}
    use_nonorthogonal_correction = true
  []

  [solid-conduction]
    type = LinearFVDiffusion
    variable = T_solid
    diffusion_coeff = ${k_s}
    use_nonorthogonal_correction = true
  []
[]

[LinearFVBCs]
# velocity BCs
  [inlet-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'ent_left'
    variable = vel_x
    functor = ${vin}
  []
  [inlet-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'ent_left'
    variable = vel_y
    functor = '0.000'
  []
  [walls-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'ent_bottom interface'
    variable = vel_x
    functor = 0.0
  []
  [walls-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'ent_bottom interface'
    variable = vel_y
    functor = 0.0
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'fluid_right'
    variable = pressure
    functor = ${P_out}
  []
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = 'fluid_right'
    variable = vel_x
    use_two_term_expansion = false
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = 'fluid_right'
    variable = vel_y
    use_two_term_expansion = false
  []

# freestream BCs for top of fluid domain
  [freestream_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = 'fluid_top ent_top'
    variable = vel_x
    use_two_term_expansion = false
  []
  [freestream_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = 'fluid_top ent_top'
    variable = vel_y
    use_two_term_expansion = false
  []
  [freestream_p]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    boundary = 'fluid_top ent_top'
    variable = pressure
    functor = 0
  []

# temperature BCs
  [inlet_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_fluid
    functor = ${Tin}
    boundary = 'ent_left'
  []
  [heated_wall_solid]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_solid
    functor = ${T_s_bottom}
    boundary = 'solid_bottom'
  []
  [insulated_fluid]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    variable = T_fluid
    functor = 0
    boundary = 'ent_top ent_bottom fluid_top fluid_right'
  []
  [insulated_solid]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    variable = T_solid
    functor = 0
    boundary = 'solid_left solid_right'
  []

# interface bcs

# pg bc
#  [fluid_solid]
#    type = LinearFVConvectiveHeatTransferBC
#    variable = T_fluid
#    T_solid = T_solid
#    T_fluid = T_fluid
#    boundary = interface
#    h = ${h_test}
#  []
#  [solid_fluid]
#    type = LinearFVConvectiveHeatTransferBC
#    variable = T_solid
#    T_solid = T_solid
#    T_fluid = T_fluid
#    boundary = interface
#    h = ${h_test}
#  []

# pg bc v2
  [fluid_solid]
    type = LinearFVConjugateHeatTransferBC
    variable = T_fluid
    T_solid = T_solid
    T_fluid = T_fluid
    boundary = interface
    h = ${h_f}
    solid_conductivity = ${k_s}
    fluid_conductivity = ${k}
  []
  [solid_fluid]
    type = LinearFVConjugateHeatTransferBC
    variable = T_solid
    T_solid = T_solid
    T_fluid = T_fluid
    boundary = interface
    h = ${h_s}
    solid_conductivity = ${k_s}
    fluid_conductivity = ${k}
  []

  [insulated_walls_T]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    variable = T_fluid
    functor = 0.0
    boundary = 'fluid_top ent_top ent_bottom solid_left solid_right'
  []
  [outlet_T]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = T_fluid
    use_two_term_expansion = false
    boundary = 'fluid_right'
  []
[]

[FunctorMaterials]
  [rhocpT]
    property_name = 'rhocpT'
    type = ParsedFunctorMaterial
    functor_names = 'T_fluid'
    expression = '${rho}*${cp}*T_fluid'
  []
[]

[Postprocessors]
  [h_in]
    type = VolumetricFlowRate
    boundary = 'ent_left'
    vel_x = vel_x
    vel_y = vel_y
    rhie_chow_user_object = rc
    advected_quantity = 'rhocpT'
    subtract_mesh_velocity = false
  []
  [h_out]
    type = VolumetricFlowRate
    boundary = 'fluid_right fluid_top ent_top interface'
    vel_x = vel_x
    vel_y = vel_y
    rhie_chow_user_object = rc
    advected_quantity = 'rhocpT'
    advected_interp_method = upwind
    subtract_mesh_velocity = false
  []
[]

[VectorPostprocessors]
  [y_vs_ts]
    type = LineValueSampler
    variable = 'T_solid'
    start_point = '0.05 ${fparse -b} 0'
    end_point = '0.05 0 0'
    num_points = 10
    sort_by = y
  []
  [y_vs_tf]
    type = LineValueSampler
    variable = 'T_fluid'
    start_point = '0.05  0 0'
    end_point = '0.05 ${fparse 2.0*b} 0'
    num_points = 11
    sort_by = y
  []
  [t_s_interface]
    type = LineValueSampler
    variable = 'T_solid'
    start_point = '0.0 -0.0001 0'
    end_point   = '${l} -0.0001 0'
    num_points = 11
    sort_by = y
    execute_on = 'final'
  []
  [t_f_interface]
    type = SideValueSampler
    variable = T_solid
    sort_by = x
    execute_on = final
    boundary = interface
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  energy_l_abs_tol = 1e-13
  solid_energy_l_abs_tol = 1e-13
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  solid_energy_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  solid_energy_system = 'solid_energy_system'
  momentum_equation_relaxation = 0.8
  energy_equation_relaxation = 1.0
  pressure_variable_relaxation = 0.3
  num_iterations = 1000
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  energy_absolute_tolerance = 1e-10
  solid_energy_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  solid_energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  solid_energy_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true
  num_cht_fpi = 1
  cht_fpi_tolerance = 1e-6
[]

[Outputs]
  exodus = true
  csv = true
  execute_on = timestep_end
[]
