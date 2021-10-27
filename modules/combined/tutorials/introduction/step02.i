#
# Three shell thermo mechanical contact
# https://mooseframework.inl.gov/modules/combined/tutorials/introduction/step02.html
#

[GlobalParams]
  displacements = 'disp_x disp_y'
  block = '0 1 2'
[]

[Problem]
  # switch to an axisymmetric coordinate system
  coord_type = RZ
[]

[Mesh]
  # inner cylinder
  [inner]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 40
    xmax = 1
    ymin = -1.75
    ymax = 1.75
    boundary_name_prefix = inner
  []

  # middle shell with subdomain ID 1
  [middle_elements]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 40
    xmin = 1.1
    xmax = 2.1
    ymin = -2.5
    ymax = 2.5
    boundary_name_prefix = middle
    boundary_id_offset = 4
  []
  [middle]
    type = SubdomainIDGenerator
    input = middle_elements
    subdomain_id = 1
  []

  # outer shell with subdomain ID 2
  [outer_elements]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 48
    xmin = 2.2
    xmax = 3.2
    ymin = -3
    ymax = 3
    boundary_name_prefix = outer
    boundary_id_offset = 8
  []
  [outer]
    type = SubdomainIDGenerator
    input = outer_elements
    subdomain_id = 2
  []

  [collect_meshes]
    type = MeshCollectionGenerator
    inputs = 'inner middle outer'
  []

  # add set of 3 nodes to remove rigid body modes for y-translation in each block
  [pin]
    type = ExtraNodesetGenerator
    input = collect_meshes
    new_boundary = pin
    coord = '0 0 0; 1.6 0 0; 2.7 0 0'
  []

  patch_update_strategy = iteration
[]

[Variables]
  # temperature field variable (first order Lagrange by default)
  [T]
  []
  # temperature lagrange multipliers
  [Tlm1]
    block = 'inner_gap_secondary_subdomain'
  []
  [Tlm2]
    block = 'outer_gap_secondary_subdomain'
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = T
  []
  [dTdt]
    type = HeatConductionTimeDerivative
    variable = T
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    eigenstrain_names = thermal
    generate_output = 'vonmises_stress stress_xx strain_xx stress_yy strain_yy'
    volumetric_locking_correction = true
    temperature = T
  []
[]

[Contact]
  [inner_gap]
    primary = middle_left
    secondary = inner_right
    model = frictionless
    formulation = mortar
    c_normal = 1e+0
  []
  [outer_gap]
    primary = outer_left
    secondary = middle_right
    model = frictionless
    formulation = mortar
    c_normal = 1e+0
  []
[]

[Constraints]
  # thermal contact constraint
  [Tlm1]
    type = GapConductanceConstraint
    variable = Tlm1
    secondary_variable = T
    use_displaced_mesh = true
    k = 1e-1
    primary_boundary = middle_left
    primary_subdomain = inner_gap_secondary_subdomain
    secondary_boundary = inner_right
    secondary_subdomain = inner_gap_primary_subdomain
  []
  [Tlm2]
    type = GapConductanceConstraint
    variable = Tlm2
    secondary_variable = T
    use_displaced_mesh = true
    k = 1e-1
    primary_boundary = outer_left
    primary_subdomain = outer_gap_secondary_subdomain
    secondary_boundary = middle_right
    secondary_subdomain = outer_gap_primary_subdomain
  []
[]

[BCs]
  [center_axis_fix]
    type = DirichletBC
    variable = disp_x
    boundary = 'inner_left'
    value = 0
  []
  [y_translation_fix]
    type = DirichletBC
    variable = disp_y
    boundary = 'pin'
    value = 0
  []
  [heat_center]
    type = FunctionDirichletBC
    variable = T
    boundary = 'inner_left'
    function = t*40
  []
  [cool_right]
    type = DirichletBC
    variable = T
    boundary = 'outer_right'
    value = 0
  []
[]

[Materials]
  [eigen_strain_inner]
    type = ComputeThermalExpansionEigenstrain
    eigenstrain_name = thermal
    temperature = T
    thermal_expansion_coeff = 1e-3
    stress_free_temperature = 0
    block = 0
  []
  [eigen_strain_middle]
    type = ComputeThermalExpansionEigenstrain
    eigenstrain_name = thermal
    temperature = T
    thermal_expansion_coeff = 2e-4
    stress_free_temperature = 0
    block = 1
  []
  [eigen_strain_outer]
    type = ComputeThermalExpansionEigenstrain
    eigenstrain_name = thermal
    temperature = T
    thermal_expansion_coeff = 1e-5
    stress_free_temperature = 0
    block = 2
  []

  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []

  # thermal properties
  [thermal_conductivity_0]
    type = HeatConductionMaterial
    thermal_conductivity = 50
    specific_heat = 1
    block = 0
  []
  [thermal_conductivity_1]
    type = HeatConductionMaterial
    thermal_conductivity = 5
    specific_heat = 1
    block = 1
  []
  [thermal_conductivity_2]
    type = HeatConductionMaterial
    thermal_conductivity = 1
    specific_heat = 1
    block = 2
  []
  [density]
    type = Density
    density = 1
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

# [Debug]
#   show_var_residual_norms = true
# []

[Executioner]
  type = Transient
  solve_type = PJFNK
  line_search = none
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       nonzero              '
  snesmf_reuse_base = false
  end_time = 7
  dt = 0.05

  nl_rel_tol = 1e-08
  nl_abs_tol = 1e-50

  [Predictor]
    type = SimplePredictor
    scale = 0.5
  []
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  perf_graph = true
[]
