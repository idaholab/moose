E_block = 1e7
E_plank = 1e7
elem = QUAD4
order = FIRST
name = 'finite'

[Mesh]
  patch_size = 80
  patch_update_strategy = auto
  [plank]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -0.3
    xmax = 0.3
    ymin = -10
    ymax = 10
    nx = 2
    ny = 67
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
    xmin = 0.31
    xmax = 0.91
    ymin = 7.7
    ymax = 8.5
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

[Problem]
  type = ReferenceResidualProblem
  reference_vector = 'ref'
  extra_tag_vectors = 'ref'
  converge_on = 'disp_x disp_y temp'
[]

[Variables]
  [disp_x]
    order = ${order}
    block = 'plank block'
    scaling = ${fparse 2.0 / (E_plank + E_block)}
  []
  [disp_y]
    order = ${order}
    block = 'plank block'
    scaling = ${fparse 2.0 / (E_plank + E_block)}
  []
  [temp]
    order = ${order}
    block = 'plank block'
    scaling = 1e-1
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [action]
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress hydrostatic_stress strain_xx strain_yy strain_zz'
    block = 'plank block'
    use_automatic_differentiation = true
    strain = FINITE
    extra_vector_tags = 'ref'
  []
[]

[Kernels]
  [hc]
    type = ADHeatConduction
    variable = temp
    use_displaced_mesh = true
    block = 'plank block'
    extra_vector_tags = 'ref'
  []
[]

[Contact]
  [frictionless]
    primary = plank_right
    secondary = block_left
    formulation = mortar
    c_normal = 1e6
  []
[]

[MortarGapHeatTransfer]
  [mortar_heat_transfer]
   temperature = temp
   use_displaced_mesh = true
   gap_flux_options = conduction
   gap_conductivity = 1
   boundary = plank_right
   primary_boundary = plank_right
   primary_subdomain = frictionless_primary_subdomain
   secondary_boundary = block_left
   secondary_subdomain = frictionless_secondary_subdomain
   thermal_lm_scaling = 1e-7
   gap_geometry_type = PLATE
  []
[]

[BCs]
  [left_temp]
    type = ADDirichletBC
    variable = temp
    boundary = 'plank_left'
    value = 400
  []

  [right_temp]
    type = ADDirichletBC
    variable = temp
    boundary = 'block_right'
    value = 300
  []

  [left_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = plank_left
    value = 0.0
  []
  [left_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = plank_bottom
    value = 0.0
  []
  [right_x]
    type = ADFunctionDirichletBC
    variable = disp_x
    boundary = block_right
    function = '-0.04*sin(4*(t+1.5))+0.02'
    preset = false
  []
  [right_y]
    type = ADFunctionDirichletBC
    variable = disp_y
    boundary = block_right
    function = '-t'
    preset = false
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

  [heat_plank]
    type = ADHeatConductionMaterial
    block = plank
    thermal_conductivity = 2
    specific_heat = 1
  []
  [heat_block]
    type = ADHeatConductionMaterial
    block = block
    thermal_conductivity = 1
    specific_heat = 1
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -snes_max_it'
  petsc_options_value = 'lu       NONZERO               1e-15                   20'
  end_time = 13.5
  dt = 0.1
  dtmin = 0.1
  timestep_tolerance = 1e-6
  line_search = 'none'
  nl_abs_tol = 1e-13
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
  [avg_temp]
    type = ElementAverageValue
    variable = temp
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
    show = 'contact avg_temp'
  []
  [out]
    type = CSV
    file_base = '${name}_out'
  []
  [dof]
    type = DOFMap
    execute_on = 'initial'
  []
[]

[Debug]
  show_var_residual_norms = true
[]
