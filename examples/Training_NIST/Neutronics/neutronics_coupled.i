# Power parameters
plate_power = 4381 # 1 GW/m3 * fuel volume per plate

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = mesh_in.e
  []
[]

[TransportSystems]
  particle = neutron
  equation_type = eigenvalue

  ReflectingBoundary = 'left right bottom top'
  VacuumBoundary = 'front back'

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
    initial_condition = 350
  []
  [tAl]
    initial_condition = 350
  []
  [bu]
    initial_condition = 0
  []
[]

[GlobalParams]
  library_file = XS_research_reactor.xml
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
[]

[MultiApps]
  [heat_conduction]
    type = FullSolveMultiApp
    input_files = heat_conduction_coupled.i
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [power_density]
    type = MultiAppCopyTransfer
    to_multi_app = heat_conduction
    source_variable = power_density
    variable = power_density
  []
  [fuel_temperature]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = heat_conduction
    source_variable = Tfuel
    variable = tf
  []
  [clad_temperature]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = heat_conduction
    source_variable = Tclad
    variable = tAl
  []
[]

[Executioner]
  type = Eigenvalue
  solve_type = PJFNKMO
  constant_matrices = true

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 100'

  nl_abs_tol = 1e-11

  fixed_point_max_its = 100
  fixed_point_rel_tol = 1e-12
  fixed_point_abs_tol = 1e-50

  custom_pp = eigenvalue
[]

[Outputs]
  exodus = true
[]
