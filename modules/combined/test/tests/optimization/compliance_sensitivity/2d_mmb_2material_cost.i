vol_frac = 0.5

power = 3

E0 = 1.0e-6
E1 = 0.3
E2 = 1.0

rho0 = 1.0e-6
rho1 = 0.3
rho2 = 1.0

C0 = 1.0e-6
C1 = 0.5
C2 = 1.0

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
    new_boundary = hold
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
[]

[AuxVariables]
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
    variable = disp_y
    boundary = hold
    value = 0.0
  []
  [no_y]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.0
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
    # ordered multimaterial simp
    expression = "A1:=(${E0}-${E1})/(${rho0}^${power}-${rho1}^${power}); "
                 "B1:=${E0}-A1*${rho0}^${power}; E1:=A1*mat_den^${power}+B1; "
                 "A2:=(${E1}-${E2})/(${rho1}^${power}-${rho2}^${power}); "
                 "B2:=${E1}-A2*${rho1}^${power}; E2:=A2*mat_den^${power}+B2; "
                 "if(mat_den<${rho1},E1,E2)"
    coupled_variables = 'mat_den'
    property_name = E_phys
  []
  [Cost]
    type = DerivativeParsedMaterial
    # ordered multimaterial simp
    expression = "A1:=(${C0}-${C1})/(${rho0}^${power}-${rho1}^${power}); "
                 "B1:=${C0}-A1*${rho0}^${power}; C1:=A1*mat_den^${power}+B1; "
                 "A2:=(${C1}-${C2})/(${rho1}^${power}-${rho2}^${power}); "
                 "B2:=${C1}-A2*${rho1}^${power}; C2:=A2*mat_den^${power}+B2; "
                 "if(mat_den<${rho1},C1,C2)"
    coupled_variables = 'mat_den'
    property_name = Cost
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
    cost = Cost
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
    radius = 1.2
    weights = linear
    prop_name = sensitivity
    execute_on = TIMESTEP_END
    force_preaux = true
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
    force_postaux = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  nl_abs_tol = 1e-8
  dt = 1.0
  num_steps = 70
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
