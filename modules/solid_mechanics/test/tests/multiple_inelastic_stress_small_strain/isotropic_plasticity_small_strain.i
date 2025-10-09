# Test for ComputeMultipleInelasticStressSmallStrain with isotropic plasticity
# This test uses the small strain (total) formulation with J2 plasticity
#
# A single element is loaded in tension in the y direction while fixing
# displacement in x and z directions. This test verifies that the small strain
# multiple inelastic stress calculator produces correct stress and plastic strain.
#
# This is similar to isotropic_plasticity_incremental_strain.i but uses
# ComputeMultipleInelasticStressSmallStrain instead of ComputeMultipleInelasticStress

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [top_pull]
    type = ParsedFunction
    expression = 't * 0.01'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    incremental = false
    add_variables = true
    generate_output = 'stress_yy elastic_strain_yy'
  []
[]

[BCs]
  [y_pull_function]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = top_pull
  []
  [x_sides]
    type = DirichletBC
    variable = disp_x
    boundary = 'left right'
    value = 0.0
  []
  [y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [z_sides]
    type = DirichletBC
    variable = disp_z
    boundary = 'back front'
    value = 0.0
  []
[]

[AuxVariables]
  [plastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [plastic_strain_yy]
    type = RankTwoAux
    rank_two_tensor = combined_inelastic_strain
    variable = plastic_strain_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.5e5
    poissons_ratio = 0.0
  []
  [isotropic_plasticity]
    type = IsotropicPlasticityStressUpdate
    yield_stress = 25.0
    hardening_constant = 1000.0
  []
  [stress]
    type = ComputeMultipleInelasticStressSmallStrain
    inelastic_models = 'isotropic_plasticity'
    tangent_operator = elastic
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 20
  nl_max_its = 20
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-9

  start_time = 0.0
  end_time = 0.01875
  dt = 0.00125
  dtmin = 0.0001
[]

[Outputs]
  exodus = true
[]
