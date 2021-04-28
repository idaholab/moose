[MultiApps]
  [disp]
    type = TransientMultiApp
    input_files = 'cohesive_mode2_picard_disp.i'
  []
[]

[Transfers]
  [to_c]
    type = MultiAppCopyTransfer
    multi_app = disp
    direction = to_multiapp
    variable = c
    source_variable = c
  []
  [to_disp_x]
    type = MultiAppCopyTransfer
    multi_app = disp
    direction = from_multiapp
    variable = disp_x
    source_variable = disp_x
  []
  [to_disp_y]
    type = MultiAppCopyTransfer
    multi_app = disp
    direction = from_multiapp
    variable = disp_y
    source_variable = disp_y
  []
[]

[Mesh]
  [top_half]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 30
    ny = 15
    ymin = 0
    ymax = 0.5
    boundary_id_offset = 0
    boundary_name_prefix = top_half
  []
  [top_stitch]
    type = BoundingBoxNodeSetGenerator
    input = top_half
    new_boundary = top_stitch
    bottom_left = '0.5 0 0'
    top_right = '1 0 0'
  []
  [bottom_half]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 30
    ny = 15
    ymin = -0.5
    ymax = 0
    boundary_id_offset = 5
    boundary_name_prefix = bottom_half
  []
  [bottom_stitch]
    type = BoundingBoxNodeSetGenerator
    input = bottom_half
    new_boundary = bottom_stitch
    bottom_left = '0.5 0 0'
    top_right = '1 0 0'
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'top_stitch bottom_stitch'
    stitch_boundaries_pairs = 'top_stitch bottom_stitch'
  []
  construct_side_list_from_node_list = true
[]

[Adaptivity]
  marker = marker
  initial_marker = marker
  initial_steps = 2
  stop_time = 0
  max_h_level = 2
  [Markers]
    [marker]
      type = OrientedBoxMarker
      center = '0.65 -0.25 0'
      length = 0.8
      width = 0.2
      height = 1
      length_direction = '1 -1.5 0'
      width_direction = '1.5 1 0'
      inside = REFINE
      outside = DO_NOTHING
    []
  []
[]

[Variables]
  [c]
  []
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
  [bounds_dummy]
  []
[]

[Bounds]
  [irreversibility]
    type = VariableOldValueBoundsAux
    variable = bounds_dummy
    bounded_variable = c
    bound_type = lower
  []
  [upper_bound]
    type = ConstantBoundsAux
    variable = bounds_dummy
    bounded_variable = c
    bound_value = 1
    bound_type = upper
  []
[]

[Kernels]
  [ACBulk]
    type = AllenCahn
    variable = c
    f_name = F
  []
  [ACInterface]
    type = ACInterface
    variable = c
    kappa_name = kappa
    mob_name = L
  []
[]

[Materials]
  # fracture
  [fracture_bulk]
    type = GenericConstantMaterial
    prop_names = ' Gc  l    c0           psic  xi'
    prop_values = '2.7 0.02 ${fparse pi} 14.88 2'
  []
  [mobility]
    type = ParsedMaterial
    f_name = L
    material_property_names = 'Gc c0 l'
    function = 'Gc/c0/l'
    constant_on = SUBDOMAIN
  []
  [interface_coef]
    type = ParsedMaterial
    f_name = kappa
    material_property_names = 'l'
    function = '2*l^2'
    constant_on = SUBDOMAIN
  []
  [degradation]
    type = DerivativeParsedMaterial
    f_name = g
    args = 'c'
    material_property_names = 'L psic xi'
    function = '(1-c)^p/((1-c)^p+(L/psic*xi)*c*(1+a2*c+a2*a3*c^2))*(1-eta)+eta'
    constant_names = '      p a2   a3 eta'
    constant_expressions = '2 -0.5 0  1e-8'
    derivative_order = 2
  []
  [crack_geometric_function]
    type = DerivativeParsedMaterial
    f_name = w
    args = 'c'
    function = '2*c-c^2'
    derivative_order = 2
  []
  [free_energy]
    type = DerivativeParsedMaterial
    f_name = F
    args = 'c'
    material_property_names = 'w(c) E_el(c) L'
    function = 'w+E_el/L'
    derivative_order = 2
  []

  # mechanics
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
    constant_on = SUBDOMAIN
  []
  [damage]
    type = PhaseFieldFractureStrainSpectralSplit
    c = c
    degradation_function = g
    elastic_energy = E_el
    use_old_elastic_energy = false
  []
  [strain]
    type = ComputeFiniteStrain
    displacements = 'disp_x disp_y'
  []
  [stress]
    type = ComputeDamageStress
    damage_model = damage
  []
[]

[Postprocessors]
  [c_norm]
    type = NodalSum
    variable = c
    outputs = none
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -snes_type'
  petsc_options_value = 'lu       superlu_dist                  vinewtonrsls'
  automatic_scaling = true

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  dt = 2e-5
  end_time = 2e-2

  picard_max_its = 20
  picard_custom_pp = c_norm
  custom_abs_tol = 0.01
  disable_picard_residual_norm_check = true
  accept_on_max_picard_iteration = true
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
