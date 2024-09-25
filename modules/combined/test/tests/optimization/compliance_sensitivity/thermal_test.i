vol_frac = 0.4
cost_frac = 10.0

power = 2.0

E0 = 1.0e-6
E1 = 1.0

rho0 = 0.0
rho1 = 1.0

C0 = 1.0e-6
C1 = 1.0

TC0 = 1.0e-16
TC1 = 1.0

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [MeshGenerator]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
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
    included_boundaries = left
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
  [temp]
    initial_condition = 100.0
  []
[]

[AuxVariables]
  [Dc]
    family = MONOMIAL
    order = FIRST
    initial_condition = -1.0
  []
  [Cc]
    family = MONOMIAL
    order = FIRST
    initial_condition = -1.0
  []
  [Tc]
    family = MONOMIAL
    order = FIRST
    initial_condition = -1.0
  []
  [Cost]
    family = MONOMIAL
    order = FIRST
    initial_condition = -1.0
  []
  [mat_den]
    family = MONOMIAL
    order = FIRST
    initial_condition = ${vol_frac}
  []
[]

[AuxKernels]
  [Cost]
    type = MaterialRealAux
    variable = Cost
    property = Cost_mat
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temp
    diffusion_coefficient = thermal_cond
  []
  [heat_source]
    type = HeatSource
    value = 1e-2 # W/m^3
    variable = temp
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
  [left_n1]
    type = DirichletBC
    variable = temp
    boundary = n1
    value = 0.0
  []
  [top]
    type = NeumannBC
    variable = temp
    boundary = top
    value = 0
  []
  [bottom]
    type = NeumannBC
    variable = temp
    boundary = bottom
    value = 0
  []
  [right]
    type = NeumannBC
    variable = temp
    boundary = right
    value = 0
  []
  [left]
    type = NeumannBC
    variable = temp
    boundary = left
    value = 0
  []
[]

[NodalKernels]
  [push_left]
    type = NodalGravity
    variable = disp_y
    boundary = push_left
    gravity_value = 0.0 # -1e-8
    mass = 1
  []
  [push_center]
    type = NodalGravity
    variable = disp_y
    boundary = push_center
    gravity_value = 0.0 # -1e-8
    mass = 1
  []
[]

[Materials]
  [thermal_cond]
    type = DerivativeParsedMaterial
    # ordered multimaterial simp
    expression = "A1:=(${TC0}-${TC1})/(${rho0}^${power}-${rho1}^${power}); "
                 "B1:=${TC0}-A1*${rho0}^${power}; TC1:=A1*mat_den^${power}+B1; TC1"
    coupled_variables = 'mat_den'
    property_name = thermal_cond
    outputs = 'exodus'
  []
  [thermal_compliance]
    type = ThermalCompliance
    temperature = temp
    thermal_conductivity = thermal_cond
    outputs = 'exodus'
  []
  [elasticity_tensor]
    type = ComputeVariableIsotropicElasticityTensor
    youngs_modulus = E_phys
    poissons_ratio = poissons_ratio
    args = 'mat_den'
  []
  [E_phys]
    type = DerivativeParsedMaterial
    # ordered multimaterial simp
    expression = "A1:=(${E0}-${E1})/(${rho0}^${power}-${rho1}^${power}); "
                 "B1:=${E0}-A1*${rho0}^${power}; E1:=A1*mat_den^${power}+B1; E1"
    coupled_variables = 'mat_den'
    property_name = E_phys
  []
  [Cost_mat]
    type = DerivativeParsedMaterial
    # ordered multimaterial simp
    expression = "A1:=(${C0}-${C1})/(${rho0}^(1/${power})-${rho1}^(1/${power})); "
                 "B1:=${C0}-A1*${rho0}^(1/${power}); C1:=A1*mat_den^(1/${power})+B1; C1"
    coupled_variables = 'mat_den'
    property_name = Cost_mat
  []
  [CostDensity]
    type = ParsedMaterial
    property_name = CostDensity
    coupled_variables = 'mat_den Cost'
    expression = 'mat_den*Cost'
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
  []
  [cc]
    type = CostSensitivity
    design_density = mat_den
    cost = Cost_mat
    outputs = 'exodus'
  []
  [tc]
    type = ThermalSensitivity
    design_density = mat_den
    thermal_conductivity = thermal_cond
    temperature = temp
    outputs = 'exodus'
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
    radius = 0.1
    weights = linear
    prop_name = sensitivity
    execute_on = TIMESTEP_END
    force_preaux = true
  []
  [rad_avg_cost]
    type = RadialAverage
    radius = 0.1
    weights = linear
    prop_name = cost_sensitivity
    execute_on = TIMESTEP_END
    force_preaux = true
  []
  [rad_avg_thermal]
    type = RadialAverage
    radius = 0.1
    weights = linear
    prop_name = thermal_sensitivity
    execute_on = TIMESTEP_END
    force_preaux = true
  []
  [update]
    type = DensityUpdateTwoConstraints
    density_sensitivity = Dc
    cost_density_sensitivity = Cc
    cost = Cost
    cost_fraction = ${cost_frac}
    design_density = mat_den
    volume_fraction = ${vol_frac}

    bisection_lower_bound = 0
    bisection_upper_bound = 1.0e12 # 100

    use_thermal_compliance = true
    thermal_sensitivity = Tc
    weight_mechanical_thermal = '0 1'

    relative_tolerance = 1.0e-12
    bisection_move = 0.015
    adaptive_move = false
    execute_on = TIMESTEP_BEGIN
  []
  # Provides Dc
  [calc_sense]
    type = SensitivityFilter
    density_sensitivity = Dc
    design_density = mat_den
    filter_UO = rad_avg
    execute_on = TIMESTEP_END
    force_postaux = true
  []
  # Provides Cc
  [calc_sense_cost]
    type = SensitivityFilter
    density_sensitivity = Cc
    design_density = mat_den
    filter_UO = rad_avg_cost
    execute_on = TIMESTEP_END
    force_postaux = true
  []
  # Provides Tc
  [calc_sense_thermal]
    type = SensitivityFilter
    density_sensitivity = Tc
    design_density = mat_den
    filter_UO = rad_avg_thermal
    execute_on = TIMESTEP_END
    force_postaux = true
  []
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  nl_abs_tol = 1e-12
  dt = 1.0
  num_steps = 5
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
  [right_flux]
    type = SideDiffusiveFluxAverage
    variable = temp
    boundary = right
    diffusivity = 10
  []
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
  []
  [cost_sensitivity]
    type = ElementIntegralMaterialProperty
    mat_prop = cost_sensitivity
  []
  [cost]
    type = ElementIntegralMaterialProperty
    mat_prop = CostDensity
  []
  [cost_frac]
    type = ParsedPostprocessor
    expression = 'cost / mesh_volume'
    pp_names = 'cost mesh_volume'
  []
  [objective]
    type = ElementIntegralMaterialProperty
    mat_prop = strain_energy_density
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [objective_thermal]
    type = ElementIntegralMaterialProperty
    mat_prop = thermal_compliance
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
