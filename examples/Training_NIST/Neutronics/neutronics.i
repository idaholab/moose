# Power parameters
plate_power = 4381 # 1 GW/m3 * fuel volume per plate

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = neutronics_mesh_in.e
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

[Executioner]
  type = Eigenvalue
  solve_type = PJFNKMO
  constant_matrices = true

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 100'
[]

[PowerDensity]
  power = ${fparse plate_power * 2}
  power_density_variable = power_density
[]

[Outputs]
  exodus = true
[]
