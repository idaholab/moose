vol_frac = 0.4

power = 2.0

E0 = 1.0e-6
E1 = 1.0

rho0 = 0.0
rho1 = 1.0


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
  [Dc]
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
    gravity_value = -1.0e-3
    mass = 1
  []
  [push_center]
    type = NodalGravity
    variable = disp_y
    boundary = push_center
    gravity_value = -1.0e-3
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
    expression = "A1:=(${E0}-${E1})/(${rho0}^${power}-${rho1}^${power}); "
                 "B1:=${E0}-A1*${rho0}^${power}; E1:=A1*mat_den^${power}+B1; E1"
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
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  nl_abs_tol = 1e-12
  dt = 1.0
  num_steps = 500
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
    execute_on = 'INITIAL TIMESTEP_END'
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
  [objective]
    type = ElementIntegralMaterialProperty
    mat_prop = strain_energy_density
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
