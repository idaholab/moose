[GlobalParams]
  displacements = 'disp_x disp_y'
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

[Modules]
  [TensorMechanics]
    [Master]
      [mech]
        strain = FINITE
        incremental = true
        additional_generate_output = 'stress_yy'
        save_in = 'resid_x resid_y'
      []
    []
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [c]
  []
[]

[AuxVariables]
  [resid_x]
  []
  [resid_y]
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
  [solid_x_offdiag]
    type = PhaseFieldFractureMechanicsOffDiag
    variable = disp_x
    c = c
    component = 0
  []
  [solid_y_offdiag]
    type = PhaseFieldFractureMechanicsOffDiag
    variable = disp_y
    c = c
    component = 1
  []
[]

[BCs]
  [xdisp]
    type = FunctionDirichletBC
    variable = 'disp_x'
    boundary = 'top_half_top'
    function = 't'
    preset = false
  []
  [yfix]
    type = DirichletBC
    variable = 'disp_y'
    boundary = 'top_half_top bottom_half_bottom'
    value = 0
  []
  [xfix]
    type = DirichletBC
    variable = 'disp_x'
    boundary = 'bottom_half_bottom'
    value = 0
  []
[]

[Materials]
  # fracture
  [fracture_bulk]
    type = GenericConstantMaterial
    prop_names = 'Gc l c0'
    prop_values = '2.7 0.02 2'
  []
  [mobility]
    type = ParsedMaterial
    f_name = L
    material_property_names = 'Gc'
    function = '1/Gc'
    constant_on = SUBDOMAIN
  []
  [interface_coef]
    type = ParsedMaterial
    f_name = kappa
    material_property_names = 'Gc l c0'
    function = '2*Gc*l/c0'
    constant_on = SUBDOMAIN
  []
  [degradation]
    type = DerivativeParsedMaterial
    f_name = g
    args = 'c'
    function = '(1-c)^2*(1-eta)+eta'
    constant_names = 'eta'
    constant_expressions = '1e-6'
    derivative_order = 2
  []
  [crack_geometric_function]
    type = DerivativeParsedMaterial
    f_name = w
    args = 'c'
    material_property_names = 'Gc l c0'
    function = 'c^2*Gc/c0/l'
    derivative_order = 2
  []
  [free_energy]
    type = DerivativeParsedMaterial
    f_name = F
    args = 'c'
    material_property_names = 'w(c) E_el(c)'
    function = 'w+E_el'
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
    c = c
    degradation_function = g
    elastic_energy = E_el
    use_old_elastic_energy = true
  []
  [stress]
    type = ComputeDamageStress
    damage_model = damage
  []
[]

[Postprocessors]
  [resid_x]
    type = NodalSum
    variable = resid_x
    boundary = top_half_top
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

  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -snes_type'
  petsc_options_value = 'lu       superlu_dist                  vinewtonrsls'
  automatic_scaling = true

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  dt = 2e-5
  dtmin = 1e-9
  num_steps = 1
  # end_time = 2e-2
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
