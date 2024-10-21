vol_frac = 0.4
cost_frac = 0.2 #0.283 # Change back to 0.4

power = 4

E0 = 1.0e-6
E1 = 0.2
E2 = 0.6
E3 = 1.0

rho0 = 1.0e-6
rho1 = 0.4
rho2 = 0.7
rho3 = 1.0

C0 = 1.0e-6
C1 = 0.5
C2 = 0.8
C3 = 1.0

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [MeshGenerator]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 50
    ny = 50
    xmin = 0
    xmax = 50
    ymin = 0
    ymax = 50
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
    coord = '25 0 0'
  []
  [push_center]
    type = ExtraNodesetGenerator
    input = push_left
    new_boundary = push_center
    coord = '50 0 0'
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxVariables]
  [Dc]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = -1.0
  []
  [Cc]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = -1.0
  []
  [Cost]
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

[AuxKernels]
  [Cost]
    type = MaterialRealAux
    variable = Cost
    property = Cost_mat
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
[NodalKernels]
  [push_left]
    type = NodalGravity
    variable = disp_y
    boundary = push_left
    gravity_value = -1e-3
    mass = 1
  []
  [push_center]
    type = NodalGravity
    variable = disp_y
    boundary = push_center
    gravity_value = -1e-3
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
    # ordered multimaterial simp
    expression = "A1:=(${E0}-${E1})/(${rho0}^${power}-${rho1}^${power}); "
                 "B1:=${E0}-A1*${rho0}^${power}; E1:=A1*mat_den^${power}+B1; "
                 "A2:=(${E1}-${E2})/(${rho1}^${power}-${rho2}^${power}); "
                 "B2:=${E1}-A2*${rho1}^${power}; E2:=A2*mat_den^${power}+B2; "
                 "A3:=(${E2}-${E3})/(${rho2}^${power}-${rho3}^${power}); "
                 "B3:=${E2}-A3*${rho2}^${power}; E3:=A3*mat_den^${power}+B3; "
                 "if(mat_den<${rho1},E1,if(mat_den<${rho2},E2,E3))"
    coupled_variables = 'mat_den'
    property_name = E_phys
  []

  [Cost_mat]
    type = DerivativeParsedMaterial
    # ordered multimaterial simp
    expression = "A1:=(${C0}-${C1})/(${rho0}^(1/${power})-${rho1}^(1/${power})); "
                 "B1:=${C0}-A1*${rho0}^(1/${power}); C1:=A1*mat_den^(1/${power})+B1; "
                 "A2:=(${C1}-${C2})/(${rho1}^(1/${power})-${rho2}^(1/${power})); "
                 "B2:=${C1}-A2*${rho1}^(1/${power}); C2:=A2*mat_den^(1/${power})+B2; "
                 "A3:=(${C2}-${C3})/(${rho2}^(1/${power})-${rho3}^(1/${power})); "
                 "B3:=${C2}-A3*${rho2}^(1/${power}); C3:=A3*mat_den^(1/${power})+B3; "
                 "if(mat_den<${rho1},C1,if(mat_den<${rho2},C2,C3))"
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
    radius = 2
    weights = linear
    prop_name = sensitivity
    execute_on = TIMESTEP_END
    force_preaux = true
  []
  [rad_avg_cost]
    type = RadialAverage
    radius = 2
    weights = linear
    prop_name = cost_sensitivity
    execute_on = TIMESTEP_END
    force_preaux = true
  []
  [update]
    type = DensityUpdateTwoConstraints
    # This is
    density_sensitivity = Dc
    cost_density_sensitivity = Cc
    cost = Cost
    cost_fraction = ${cost_frac}
    design_density = mat_den
    volume_fraction = ${vol_frac}

    bisection_lower_bound = 0
    bisection_upper_bound = 1.0e16 # 100

    relative_tolerance = 1.0e-3
    bisection_move = 0.02
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
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  nl_abs_tol = 1e-10
  dt = 1.0
  num_steps = 25
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
[]
