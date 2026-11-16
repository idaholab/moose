num_elems = 11
elem_width = ${fparse 100/num_elems}
sample_start = ${fparse 100+elem_width/2}
sample_end = ${fparse 200-elem_width/2}
[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  coord_type = RZ

  # x = radial coordinate r
  # y = axial coordinate z
  xmin = 100.0
  xmax = 200.0
  ymin = 0.0
  ymax = 25.0

  # Use NAFEMS-like coarse mesh first.
  # Increase to nx = 80 only after the stress trends are correct.
  nx = 11
  ny = 2

  elem_type = QUAD8
[]

[AuxVariables]
  [sigma_rr_recovered]
    order = SECOND
    family = LAGRANGE
  []

  [sigma_zz_recovered]
    order = SECOND
    family = LAGRANGE
  []

  [sigma_hoop_recovered]
    order = SECOND
    family = LAGRANGE
  []

  [sigma_rr]
    order = CONSTANT
    family = MONOMIAL
  []

  [sigma_zz]
    order = CONSTANT
    family = MONOMIAL
  []

  [sigma_hoop]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  # In RZ:
  # index 0,0 = sigma_rr
  # index 1,1 = sigma_zz axial
  # index 2,2 = sigma_theta-theta hoop

  [sigma_rr_aux]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = sigma_rr
    index_i = 0
    index_j = 0
    execute_on = 'initial timestep_end final'
  []

  [sigma_zz_aux]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = sigma_zz
    index_i = 1
    index_j = 1
    execute_on = 'initial timestep_end final'
  []

  [sigma_hoop_aux]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = sigma_hoop
    index_i = 2
    index_j = 2
    execute_on = 'initial timestep_end final'
  []

  [sigma_rr_recovered]
    type = NodalPatchRecoveryAux
    variable = sigma_rr_recovered
    nodal_patch_recovery_uo = sigma_rr_patch
    execute_on = 'TIMESTEP_END'
  []
  [sigma_hoop_recovered]
    type = NodalPatchRecoveryAux
    variable = sigma_hoop_recovered
    nodal_patch_recovery_uo = sigma_hoop_patch
    execute_on = 'TIMESTEP_END'
  []
  [sigma_zz_recovered]
    type = NodalPatchRecoveryAux
    variable = sigma_zz_recovered
    nodal_patch_recovery_uo = sigma_zz_patch
    execute_on = 'TIMESTEP_END'
  []
[]

[UserObjects]
  [sigma_rr_patch]
    type = NodalPatchRecoveryMaterialProperty
    patch_polynomial_order = SECOND
    property = 'stress'
    component = '0 0'
    execute_on = 'TIMESTEP_END'
  []
  [sigma_hoop_patch]
    type = NodalPatchRecoveryMaterialProperty
    patch_polynomial_order = SECOND
    property = 'stress'
    component = '2 2'
    execute_on = 'TIMESTEP_END'
  []
  [sigma_zz_patch]
    type = NodalPatchRecoveryMaterialProperty
    patch_polynomial_order = SECOND
    property = 'stress'
    component = '1 1'
    execute_on = 'TIMESTEP_END'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true

    strain = SMALL # FINITE # SMALL
    incremental = false #true
  []
[]

[BCs]
  # Prescribed zero z-displacement on AB: z = 0
  [fix_AB_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  []

  # Prescribed zero z-displacement on CD: z = H
  [fix_CD_z]
    type = DirichletBC
    variable = disp_z
    boundary = top
    value = 0.0
  []

#  # Internal pressure on AD: r = R1.
#  #
#  # Pressure is applied through the standard pressure boundary condition.
#  # Positive factor gives the usual internal-pressure direction on the
#  # inner boundary in RZ.
  [inner_pressure]
    type = Pressure
    variable = disp_r
    component = 0
    boundary = left
    factor = 200.0
    use_displaced_mesh = false
  []
  # [inner_pressure]
  #   type = NeumannBC
  #   variable = disp_r
  #   boundary = left
  #   value = 200.0
  # []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 200e3
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
  []

#  [elastic_stress]
#    type = ComputeFiniteStrainElasticStress
#  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [current_dt]
    type = TimestepSize
  []

  [avg_sigma_rr]
    type = ElementAverageValue
    variable = sigma_rr_recovered
  []

  [avg_sigma_zz]
    type = ElementAverageValue
    variable = sigma_zz_recovered
  []

  [avg_sigma_hoop]
    type = ElementAverageValue
    variable = sigma_hoop_recovered
  []

  # Diagnostics
  [inner_sigma_rr]
    type = SideAverageValue
    variable = sigma_rr
    boundary = left
  []

  [outer_sigma_rr]
    type = SideAverageValue
    variable = sigma_rr
    boundary = right
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  start_time = 0.0
  end_time = 1.0
  dt = 1.0

  dtmin = 1e-12

  nl_max_its = 50
  l_max_its = 100

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-12

  automatic_scaling = true
[]

[VectorPostprocessors]
  # Output final radial profile.
  #
  # sigma_rr   -> radial stress
  # sigma_hoop -> hoop stress
  # sigma_zz   -> axial stress
  [stress_profile]
    type = LineValueSampler
    variable = 'sigma_rr_recovered sigma_hoop_recovered sigma_zz_recovered'
    start_point = '${sample_start} 12.5 0.0'
    end_point = '${sample_end} 12.5 0.0'
    num_points = ${num_elems}
    sort_by = x
    execute_on = final
    outputs = line_csv
  []

  # Same variables at the five comparison radii.
  [stress_points]
    type = LineValueSampler
    variable = 'sigma_rr_recovered sigma_hoop_recovered sigma_zz_recovered'
    start_point = '100.0 12.5 0.0'
    end_point = '200.0 12.5 0.0'
    num_points = 5
    sort_by = x
    execute_on = final
    outputs = point_csv
  []
[]

[Outputs]
  exodus = true
  [console]
    type = Console
    max_rows = 5
  []

  [csv]
    type = CSV
    execute_on = 'initial timestep_end final'
    show = '
      avg_sigma_rr
      avg_sigma_hoop
    '
  []

  [line_csv]
    type = CSV
    execute_on = final
    create_final_symlink = true
    file_base = '7_elastic_profile'
  []

  [point_csv]
    type = CSV
    execute_on = final
    create_final_symlink = true
    file_base = '7_elastic_points'
  []
[]
