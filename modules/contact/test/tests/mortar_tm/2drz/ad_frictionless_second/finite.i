E_block = 1e7
E_plank = 1e7
elem = QUAD9
order = SECOND
name = 'finite'

[Problem]
  coord_type = RZ
[]

[Mesh]
  patch_size = 80
  patch_update_strategy = auto
  [plank]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 0.6
    ymin = 0
    ymax = 10
    nx = 2
    ny = 33
    elem_type = ${elem}
    boundary_name_prefix = plank
  []
  [plank_id]
    type = SubdomainIDGenerator
    input = plank
    subdomain_id = 1
  []

  [block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.61
    xmax = 1.21
    ymin = 9.2
    ymax = 10.0
    nx = 3
    ny = 4
    elem_type = ${elem}
    boundary_name_prefix = block
    boundary_id_offset = 10
  []
  [block_id]
    type = SubdomainIDGenerator
    input = block
    subdomain_id = 2
  []

  [combined]
    type = MeshCollectionGenerator
    inputs = 'plank_id block_id'
  []
  [block_rename]
    type = RenameBlockGenerator
    input = combined
    old_block = '1 2'
    new_block = 'plank block'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [disp_x]
    order = ${order}
    block = 'plank block'
    scaling = '${fparse 2.0 / (E_plank + E_block)}'
  []
  [disp_y]
    order = ${order}
    block = 'plank block'
    scaling = '${fparse 2.0 / (E_plank + E_block)}'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [block]
    use_automatic_differentiation = true
    strain = FINITE
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress hydrostatic_stress strain_xx '
                      'strain_yy strain_zz'
    block = 'block'
  []
  [plank]
    use_automatic_differentiation = true
    strain = FINITE
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress hydrostatic_stress strain_xx '
                      'strain_yy strain_zz'
    block = 'plank'
    eigenstrain_names = 'swell'
  []
[]

[Contact]
  [frictionless]
    primary = plank_right
    secondary = block_left
    formulation = mortar
    c_normal = 1e0
  []
[]

[BCs]
  [left_x]
    type = DirichletBC
    variable = disp_x
    preset = false
    boundary = plank_left
    value = 0.0
  []
  [left_y]
    type = DirichletBC
    variable = disp_y
    preset = false
    boundary = plank_bottom
    value = 0.0
  []
  [right_x]
    type = DirichletBC
    variable = disp_x
    preset = false
    boundary = block_right
    value = 0
  []
  [right_y]
    type = ADFunctionDirichletBC
    variable = disp_y
    preset = false
    boundary = block_right
    function = '-t'
  []
[]

[Materials]
  [plank]
    type = ADComputeIsotropicElasticityTensor
    block = 'plank'
    poissons_ratio = 0.3
    youngs_modulus = ${E_plank}
  []
  [block]
    type = ADComputeIsotropicElasticityTensor
    block = 'block'
    poissons_ratio = 0.3
    youngs_modulus = ${E_block}
  []
  [stress]
    type = ADComputeFiniteStrainElasticStress
    block = 'plank block'
  []
  [swell]
    type = ADComputeEigenstrain
    block = 'plank'
    eigenstrain_name = swell
    eigen_base = '1 0 0 0 0 0 0 0 0'
    prefactor = swell_mat
  []
  [swell_mat]
    type = ADGenericFunctionMaterial
    prop_names = 'swell_mat'
    prop_values = '7e-2*(1-cos(4*t))'
    block = 'plank'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type -mat_mffd_err -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       1e-5          NONZERO               1e-15'
  end_time = 3
  dt = 0.1
  dtmin = 0.1
  timestep_tolerance = 1e-6
  line_search = 'contact'
[]

[Postprocessors]
  [nl_its]
    type = NumNonlinearIterations
  []
  [total_nl_its]
    type = CumulativeValuePostprocessor
    postprocessor = nl_its
  []
  [l_its]
    type = NumLinearIterations
  []
  [total_l_its]
    type = CumulativeValuePostprocessor
    postprocessor = l_its
  []
  [contact]
    type = ContactDOFSetSize
    variable = frictionless_normal_lm
    subdomain = frictionless_secondary_subdomain
  []
  [avg_hydro]
    type = ElementAverageValue
    variable = hydrostatic_stress
    block = 'block'
  []
  [max_hydro]
    type = ElementExtremeValue
    variable = hydrostatic_stress
    block = 'block'
  []
  [min_hydro]
    type = ElementExtremeValue
    variable = hydrostatic_stress
    block = 'block'
    value_type = min
  []
  [avg_vonmises]
    type = ElementAverageValue
    variable = vonmises_stress
    block = 'block'
  []
  [max_vonmises]
    type = ElementExtremeValue
    variable = vonmises_stress
    block = 'block'
  []
  [min_vonmises]
    type = ElementExtremeValue
    variable = vonmises_stress
    block = 'block'
    value_type = min
  []
[]

[Outputs]
  file_base = ${name}
  [comp]
    type = CSV
    show = 'contact'
  []
  [out]
    type = CSV
    file_base = '${name}_out'
  []
[]

[Debug]
  show_var_residual_norms = true
[]
