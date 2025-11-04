vol_frac = 0.5
E0 = 1e5
Emin = 1e-2
power = 2

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [MeshGenerator]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 24
    ny = 12
    nz = 12
    xmin = 0
    xmax = 20
    ymin = 0
    ymax = 10
    zmin = 0
    zmax = 10
  []
  [middle_bottom_left_edge]
    type = ExtraNodesetGenerator
    input = MeshGenerator
    new_boundary = pull
    coord = '0 0 5'
  []
[]

[AuxVariables]
  [compliance]
    family = MONOMIAL
    order = CONSTANT
  []
  [Dc]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = -1.0
  []
  [mat_den]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = ${vol_frac}
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
  [no_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.0
  []
  [no_y]
    type = DirichletBC
    variable = disp_y
    boundary = right
    value = 0.0
  []
  [no_z]
    type = DirichletBC
    variable = disp_z
    boundary = right
    value = 0.0
  []
[]
[NodalKernels]
  [pull]
    type = NodalGravity
    variable = disp_y
    boundary = pull
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
  [rad_avg]
    type = RadialAverage
    radius = 0.5
    weights = constant
    prop_name = sensitivity
    execute_on = TIMESTEP_END
    execution_order_group = -1
  []
  [update]
    type = DensityUpdate
    density_sensitivity = Dc
    design_density = mat_den
    volume_fraction = ${vol_frac}
    execute_on = TIMESTEP_BEGIN
  []
  [calc_sense]
    type = SensitivityFilter
    density_sensitivity = Dc
    design_density = mat_den
    filter_UO = rad_avg
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu '
  nl_abs_tol = 1e-10
  l_max_its = 200
  start_time = 0.0
  dt = 1.0
  num_steps = 10

[]

[Outputs]
  [out]
    type = Exodus
    time_step_interval = 10
  []
[]

