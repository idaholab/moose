[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 30
    ny = 15
    ymax = 0.5
  []
  [noncrack]
    type = BoundingBoxNodeSetGenerator
    input = gen
    new_boundary = noncrack
    bottom_left = '0.5 0 0'
    top_right = '1 0 0'
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
      type = BoxMarker
      bottom_left = '0.4 0 0'
      top_right = '1 0.05 0'
      outside = DO_NOTHING
      inside = REFINE
    []
  []
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
  [ydisp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = 't'
  []
  [yfix]
    type = DirichletBC
    variable = disp_y
    boundary = noncrack
    value = 0
  []
  [xfix]
    type = DirichletBC
    variable = disp_x
    boundary = top
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
    constant_expressions = '1e-8'
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
    type = PhaseFieldFractureStrainVolDevSplit
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
  [resid_y]
    type = NodalSum
    variable = resid_y
    boundary = top
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

  dt = 1e-5
  dtmin = 1e-9
  end_time = 4e-3
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
