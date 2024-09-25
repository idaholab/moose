vol_frac = 0.4

power = 2.0

E0 = 1.0e-6
E1 = 1.0

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [MeshGenerator]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 40
    ny = 40
    xmin = 0
    xmax = 40
    ymin = 0
    ymax = 40
  []
  [node]
    type = ExtraNodesetGenerator
    input = MeshGenerator
    new_boundary = hold
    nodes = 0
  []
  [push_left]
    type = ExtraNodesetGenerator
    input = node
    new_boundary = push_left
    coord = '16 0 0'
  []
  [push_center]
    type = ExtraNodesetGenerator
    input = push_left
    new_boundary = push_center
    coord = '24 0 0'
  []
  [extra]
    type = SideSetsFromBoundingBoxGenerator
    input = push_center
    bottom_left = '-0.01 17.999  0'
    top_right = '5 22.001  0'
    boundary_new = n1
    boundaries_old = left
  []
  [dirichlet_bc]
    type = SideSetsFromNodeSetsGenerator
    input = extra
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxVariables]
  [mat_den]
    family = MONOMIAL
    order = FIRST
    initial_condition = 0.02
  []
  [sensitivity_one]
    family = MONOMIAL
    order = FIRST
    initial_condition = -1.0
  []
  [sensitivity_two]
    family = MONOMIAL
    order = FIRST
    initial_condition = -1.0
  []
  [total_sensitivity]
    family = MONOMIAL
    order = FIRST
    initial_condition = -1.0
  []
[]

[AuxKernels]
  [total_sensitivity]
    type = ParsedAux
    variable = total_sensitivity
    expression = '(1-1.0e-7)*sensitivity_one + 1.0e-7*sensitivity_two'
    coupled_variables = 'sensitivity_one sensitivity_two'
    execute_on = 'LINEAR TIMESTEP_END'
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
    boundary = hold
    value = 0.0
  []
  [no_x_symm]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeVariableIsotropicElasticityTensor
    youngs_modulus = E_phys
    poissons_ratio = poissons_ratio
    args = 'mat_den'
  []
  [E_phys]
    type = DerivativeParsedMaterial
    # Emin + (density^penal) * (E0 - Emin)
    expression = '${E1} + (mat_den ^ ${power}) * (${E1}-${E0})'
    coupled_variables = 'mat_den'
    property_name = E_phys
  []
  [poissons_ratio]
    type = GenericConstantMaterial
    prop_names = poissons_ratio
    prop_values = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[UserObjects]
  # We do filtering in the subapps
  [update]
    type = DensityUpdate
    density_sensitivity = total_sensitivity
    design_density = mat_den
    volume_fraction = ${vol_frac}
    execute_on = MULTIAPP_FIXED_POINT_BEGIN
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  nl_abs_tol = 1e-8
  dt = 1.0
  num_steps = 2
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'TIMESTEP_END'
  []
  print_linear_residuals = false
  exodus = true
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
    type = ElementIntegralVariablePostprocessor
    variable = total_sensitivity
  []
[]

[MultiApps]
  [sub_app_one]
    type = TransientMultiApp
    input_files = structural_sub.i
  []
  [sub_app_two]
    type = TransientMultiApp
    input_files = thermal_sub.i
  []
[]

[Transfers]
  # First SUB-APP: STRUCTURAL
  # To subapp densities
  [subapp_one_density]
    type = MultiAppCopyTransfer
    to_multi_app = sub_app_one
    source_variable = mat_den # Here
    variable = mat_den
  []
  # From subapp sensitivity
  [subapp_one_sensitivity]
    type = MultiAppCopyTransfer
    from_multi_app = sub_app_one
    source_variable = Dc # sensitivity_var
    variable = sensitivity_one # Here
  []
  # Second SUB-APP: HEAT CONDUCTIVITY
  # To subapp densities
  [subapp_two_density]
    type = MultiAppCopyTransfer
    to_multi_app = sub_app_two
    source_variable = mat_den # Here
    variable = mat_den
  []
  # From subapp sensitivity
  [subapp_two_sensitivity]
    type = MultiAppCopyTransfer
    from_multi_app = sub_app_two
    source_variable = Tc # sensitivity_var
    variable = sensitivity_two # Here
  []
[]
