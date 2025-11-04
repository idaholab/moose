vol_frac = 0.35

power = 1.1

Emin = 1.0e-6
Ess = 0.475 # ss
Et = 1.0 # w

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  # final_generator = 'MoveRight'
  [Bottom]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 320
    ny = 30
    xmin = 0
    xmax = 150
    ymin = 0
    ymax = 15
  []
  [RenameBottom]
    type = RenameBoundaryGenerator
    input = Bottom
    old_boundary = 'top bottom right left'
    new_boundary = 'top_bottom bottom_bottom right_bottom left_bottom'
  []
  [Middle]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 320
    ny = 6
    xmin = 0
    xmax = 150
    ymin = 0
    ymax = 3
  []
  [MoveMiddle]
    type = TransformGenerator
    input = Middle
    transform = TRANSLATE
    vector_value = '0 15 0'
  []
  [RenameMiddle]
    type = RenameBoundaryGenerator
    input = MoveMiddle
    old_boundary = 'top bottom right left'
    new_boundary = 'top_middle bottom_middle right_middle left_middle'
  []
  [Top]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 320
    ny = 30
    xmin = 0
    xmax = 150
    ymin = 0
    ymax = 15
  []
  [MoveTop]
    type = TransformGenerator
    input = Top
    transform = TRANSLATE
    vector_value = '0 18 0'
  []
  [RenameTop]
    type = RenameBoundaryGenerator
    input = MoveTop
    old_boundary = 'top bottom right left'
    new_boundary = 'top_top bottom_top right_top left_top'
  []
  [bottom_gen]
    type = ParsedSubdomainMeshGenerator
    input = RenameBottom
    combinatorial_geometry = 'y <= 15'
    block_id = 1
  []
  [middle_gen]
    type = ParsedSubdomainMeshGenerator
    input = RenameMiddle
    combinatorial_geometry = 'y <= 18 & y > 15'
    block_id = 2
  []
  [top_gen]
    type = ParsedSubdomainMeshGenerator
    input = RenameTop
    combinatorial_geometry = 'y > 18'
    block_id = 3
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'bottom_gen middle_gen top_gen'
    stitch_boundaries_pairs = 'top_bottom bottom_middle; top_middle bottom_top'
  []
  [left_load]
    type = ExtraNodesetGenerator
    input = stitch
    new_boundary = left_load
    coord = '37.5 33 0'
  []
  [right_load]
    type = ExtraNodesetGenerator
    input = left_load
    new_boundary = right_load
    coord = '112.5 33 0'
  []
  [left_support]
    type = ExtraNodesetGenerator
    input = right_load
    new_boundary = left_support
    coord = '0 0 0'
  []
  [right_support]
    type = ExtraNodesetGenerator
    input = left_support
    new_boundary = right_support
    coord = '150 0 0'
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [Dc]
    initial_condition = -1.0
  []
[]

[AuxVariables]
  [Cc]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = -1.0
  []
  [mat_den]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = ${vol_frac}
  []

  [sensitivity]
    family = MONOMIAL
    order = FIRST
    initial_condition = -1.0
    [AuxKernel]
      type = MaterialRealAux
      variable = sensitivity
      property = sensitivity
      execute_on = LINEAR
    []
    block = '1 2 3'
  []
  [mat_den_nodal]
    family = L2_LAGRANGE
    order = FIRST
    initial_condition = ${vol_frac}
    [AuxKernel]
      type = SelfAux
      execute_on = TIMESTEP_END
      variable = mat_den_nodal
      v = mat_den
    []
  []
  [Dc_elem]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = -1.0
    [AuxKernel]
      type = SelfAux
      variable = Dc_elem
      v = Dc
      execute_on = 'TIMESTEP_END'
    []
  []
[]

[Kernels]
  [diffusion]
    type = FunctionDiffusion
    variable = Dc
    function = 4.0
  []
  [potential]
    type = Reaction
    variable = Dc
  []
  [source]
    type = CoupledForce
    variable = Dc
    v = sensitivity
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    add_variables = true
    incremental = false
  []
[]

[BCs]
  [no_y]
    type = DirichletBC
    variable = disp_y
    boundary = left_support
    value = 0.0
  []
  [no_x]
    type = DirichletBC
    variable = disp_x
    boundary = left_support
    value = 0.0
  []
  [no_y_right]
    type = DirichletBC
    variable = disp_y
    boundary = right_support
    value = 0.0
  []
  [boundary_penalty]
    type = ADRobinBC
    variable = Dc
    boundary = 'bottom_bottom right_bottom left_bottom top_top right_top left_top left_middle '
               'right_middle'
    coefficient = 10
  []
[]

[NodalKernels]
  [left_down]
    type = NodalGravity
    variable = disp_y
    boundary = left_load
    gravity_value = -1e-3
    mass = 1
  []
  [right_down]
    type = NodalGravity
    variable = disp_y
    boundary = right_load
    gravity_value = -1e-3
    mass = 1
  []
[]

[Materials]
  [sensitivity]
    type = ParsedMaterial
    property_name = 'sensitivity'
    block = '2'
    expression = '0'
  []
  [elasticity_tensor_one]
    type = ComputeVariableIsotropicElasticityTensor
    youngs_modulus = E_phys_one
    poissons_ratio = poissons_ratio
    args = 'mat_den'
    block = '1'
  []
  [elasticity_tensor_three]
    type = ComputeVariableIsotropicElasticityTensor
    youngs_modulus = E_phys_three
    poissons_ratio = poissons_ratio
    args = 'mat_den'
    block = '3'
  []
  [elasticity_tensor_two]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1.0
    poissons_ratio = 0.3
    block = '2'
  []
  # One: Tungsten
  [E_phys_one]
    type = DerivativeParsedMaterial
    # Emin + (density^penal) * (E0 - Emin)
    expression = '${Emin} + (mat_den ^ ${power}) * (${Et}-${Emin})'
    coupled_variables = 'mat_den'
    property_name = E_phys_one
    block = '1'
    outputs = 'exodus'
  []
  # Three: SS316
  [E_phys_three]
    type = DerivativeParsedMaterial
    # Emin + (density^penal) * (E0 - Emin)
    expression = '${Emin} + (mat_den ^ ${power}) * (${Ess}-${Emin})'
    coupled_variables = 'mat_den'
    property_name = E_phys_three
    block = '3'
    outputs = 'exodus'
  []
  [poissons_ratio]
    type = GenericConstantMaterial
    prop_names = poissons_ratio
    prop_values = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [dc_one]
    type = ComplianceSensitivity
    design_density = mat_den
    youngs_modulus = E_phys_one
    block = '1'
  []
  [dc_three]
    type = ComplianceSensitivity
    design_density = mat_den
    youngs_modulus = E_phys_three
    block = '3'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[UserObjects]
  [update_one]
    type = DensityUpdate
    density_sensitivity = Dc_elem
    design_density = mat_den
    volume_fraction = ${vol_frac}
    execute_on = TIMESTEP_BEGIN
    block = '1'
  []
  [update_three]
    type = DensityUpdate
    density_sensitivity = Dc_elem
    design_density = mat_den
    volume_fraction = ${vol_frac}
    execute_on = TIMESTEP_BEGIN
    block = '3'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  nl_abs_tol = 1e-10
  dt = 1.0
  num_steps = 90
[]

[Outputs]
  exodus = true
  [out]
    type = CSV
    execute_on = 'TIMESTEP_END'
  []
  print_linear_residuals = false
[]

[Postprocessors]
  [mesh_volume]
    type = VolumePostprocessor
    execute_on = 'initial timestep_end'
  []
  [total_vol]
    type = ElementIntegralVariablePostprocessor
    variable = mat_den
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [vol_frac]
    type = ParsedPostprocessor
    expression = 'total_vol / mesh_volume'
    pp_names = 'total_vol mesh_volume'
  []
  [sensitivity]
    type = ElementIntegralMaterialProperty
    mat_prop = sensitivity
    block = '1 3'
  []
  [objective_one]
    type = ElementIntegralMaterialProperty
    mat_prop = strain_energy_density
    execute_on = 'INITIAL TIMESTEP_END'
    block = '1'
  []
  [objective_three]
    type = ElementIntegralMaterialProperty
    mat_prop = strain_energy_density
    execute_on = 'INITIAL TIMESTEP_END'
    block = '3'
  []
[]
