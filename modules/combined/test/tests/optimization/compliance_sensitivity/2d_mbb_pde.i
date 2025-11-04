vol_frac = 0.5
E0 = 1
Emin = 1e-8
power = 3
[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [MeshGenerator]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 150
    ny = 50
    xmin = 0
    xmax = 30
    ymin = 0
    ymax = 10
  []
  [node]
    type = ExtraNodesetGenerator
    input = MeshGenerator
    new_boundary = hold_y
    nodes = 0
  []
  [push]
    type = ExtraNodesetGenerator
    input = node
    new_boundary = push
    coord = '30 10 0'
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
  []
  [mat_den]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = ${vol_frac}
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

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    add_variables = true
    incremental = false
  []
[]
[Kernels]
  [diffusion]
    type = FunctionDiffusion
    variable = Dc
    function = 0.15 # radius coeff
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
[BCs]
  [no_x]
    type = DirichletBC
    variable = disp_y
    boundary = hold_y
    value = 0.0
  []
  [no_y]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.0
  []
  [boundary_penalty]
    type = ADRobinBC
    variable = Dc
    boundary = 'left top'
    coefficient = 10
  []
  [boundary_penalty_right]
    type = ADRobinBC
    variable = Dc
    boundary = 'right'
    coefficient = 10
  []
[]
[NodalKernels]
  [push]
    type = NodalGravity
    variable = disp_y
    boundary = push
    gravity_value = -1
    mass = 1
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
    expression = '${Emin} + (mat_den ^ ${power}) * (${E0}-${Emin})'
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
  [dc]
    type = ComplianceSensitivity
    design_density = mat_den
    youngs_modulus = E_phys
    incremental = false
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]
[UserObjects]
  [update]
    type = DensityUpdate
    density_sensitivity = Dc_elem
    design_density = mat_den
    volume_fraction = ${vol_frac}
    execute_on = TIMESTEP_BEGIN
    force_postaux = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  line_search = none
  nl_abs_tol = 1e-4
  l_max_its = 200
  start_time = 0.0
  dt = 1.0
  num_steps = 70
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'INITIAL TIMESTEP_END'
  []
  print_linear_residuals = false
[]

[Postprocessors]
  [total_vol]
    type = ElementIntegralVariablePostprocessor
    variable = mat_den
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [sensitivity]
    type = ElementIntegralMaterialProperty
    mat_prop = sensitivity
  []
[]

[Controls]
  [first_period]
    type = TimePeriod
    start_time = 0.0
    end_time = 10
    enable_objects = 'BCs::boundary_penalty_right'
    execute_on = 'initial timestep_begin'
  []
[]
