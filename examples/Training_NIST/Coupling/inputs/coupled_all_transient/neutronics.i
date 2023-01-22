# Power parameters
plate_power = 4381 # 1 GW/m3 * fuel volume per plate

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../coupled_all_steady/neutronics_restart.e'
    use_for_exodus_restart = true
  []
[]

[TransportSystems]
  particle = neutron
  equation_type = transient
  restart_transport_system = true

  ReflectingBoundary = 'left right bottom top'
  VacuumBoundary = 'fluid-inlet fluid-outlet'

  G = 2

  [diff]
    scheme = CFEM-Diffusion
    n_delay_groups = 8

    assemble_scattering_jacobian = true
    assemble_fission_jacobian = true
    assemble_delay_jacobian = true
  []
[]

[AuxVariables]
  [tf]
    initial_from_file_var = tf
  []
  [tAl]
    initial_from_file_var = tAl
  []
  [bu]
    initial_condition = 0
  []
[]

[GlobalParams]
  library_file = '../coupled_all_steady/XS_research_reactor.xml'
  library_name = serpent_pbr_xs

  grid_names = 'tf tAl bu'
  grid_variables = 'tf tAl bu'
  isotopes = pseudo
  densities = 1.0

  is_meter = true
[]

[Materials]
  [water]
    type = CoupledFeedbackNeutronicsMaterial
    block = 'water'
    material_id = 3
  []
  [clad]
    type = CoupledFeedbackNeutronicsMaterial
    block = 'clad'
    material_id = 2
  []
  [fuel]
    type = CoupledFeedbackNeutronicsMaterial
    block = 'fuel'
    material_id = 1
    plus = true
  []
[]

[PowerDensity]
  power = ${fparse plate_power * 2}
  power_density_variable = power_density
  integrated_power_postprocessor = power
  power_scaling_postprocessor = power_scaling
  family = MONOMIAL
  order = CONSTANT
[]

[Executioner]
  type = Transient
  scheme = implicit-euler
  start_time = 0.0
  end_time = 10.0
  dt = 0.25
  solve_type = PJFNK
  petsc_options_iname = '-ksp_gmres_restart -pc_type  -pc_use_amat
                         -pc_gamg_sym_graph -pc_mg_levels  -pc_gamg_use_parallel_coarse_grid_solver
                         -mg_levels_1_pc_type -mg_coarse_pc_type
                         -mg_coarse_sub_pc_factor_levels -mg_coarse_sub_pc_factor_mat_ordering_type'
  petsc_options_value = ' 100   gamg  false
                          true   2      1
                          asm   asm
                          1     rcm'

  line_search = 'none'
  nl_abs_tol = 1e-7
  nl_forced_its = 1
  l_abs_tol = 1e-7
  l_max_its = 200

  # Fixed point iteration parameters
  fixed_point_max_its = 3
  accept_on_max_fixed_point_iteration = true
  fixed_point_abs_tol = 1e-10
[]

[MultiApps]
  [TH]
    type = TransientMultiApp
    input_files = 'TH.i'
    execute_on = TIMESTEP_END
    sub_cycling = true
  []
[]

[Transfers]
  [power_density]
    type = MultiAppCopyTransfer
    to_multi_app = TH
    source_variable = power_density
    variable = power_density
    execute_on = 'TIMESTEP_END'
  []
  [fuel_temperature]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = TH
    source_variable = Tfuel
    variable = tf
  []
  [clad_temperature]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = TH
    source_variable = Tclad
    variable = tAl
  []
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = true
  csv = true
[]
