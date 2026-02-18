E = 2.1e5  #MPa
nu = 0.3

[MultiApps]
  [fracture]
    type = TransientMultiApp
    input_files = sub.i
    execute_on = 'TIMESTEP_END'
    clone_parent_mesh = false
  []
[]

# [Transfers]
#   [from_d]
#     type = MultiAppCopyTransfer
#     from_multi_app = 'fracture'
#     variable = d
#     source_variable = d
#   []
#   [to_fracture]
#     type = MultiAppCopyTransfer
#     to_multi_app = 'fracture'
#     variable = 'SED f'
#     source_variable = 'SED f'
#   []
# []

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxVariables]
  [d]
    initial_condition= '1'
  []
  [target_d_avg]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [solid_x]
    type = ADStressDivergenceTensors
    variable = disp_x
    component = 0
  []
  [solid_y]
    type = ADStressDivergenceTensors
    variable = disp_y
    component = 1
  []
[]

[AuxKernels]
  [target_d_avg]
    type = ADMaterialRealAux
    variable = target_d_avg
    property = target_d
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Functions]
  [base_u_t]
    type = PiecewiseLinear
    x = '0 2e-6 4e-6'
    y = '0 1e-1 0'
  []
[]

[BCs]
  [ydisp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = base_u_t
  []
  [yfix]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [xfix]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0
  []
[]

[Materials]
  [strain]
    type = ADComputeSmallStrain
  []
  [elasticity]
    type = ADComputeIsotropicElasticityTensor
    poissons_ratio = ${nu}
    youngs_modulus = ${E}
  []
  [stress]
    type = ADComputeLinearElasticStress
  []
  [strain_energy_density]
    type = ADStrainEnergyDensity
  []
  [converter_to_ad]
    type = MaterialADConverter
    reg_props_in = 'strain_energy_density'
    ad_props_out = 'ad_strain_energy_density'
  []
  [SED]
    type = ADParsedMaterial
    property_name = SED
    expression = 'ad_strain_energy_density'
    material_property_names = 'ad_strain_energy_density'
    outputs = exodus
  []
  [SED_deg]
    type = ADParsedMaterial
    property_name = SED_deg
    expression = '(1-d)^2*SED'
    material_property_names = 'SED'
    coupled_variables = 'd'
    outputs = exodus
  []
  [f]
    type = ADParsedMaterial
    property_name = f
    expression = '1000/SED_deg'
    material_property_names = SED_deg
    coupled_variables = 'd'
    outputs = exodus
  []
  [target]
    type = ADParsedMaterial
    property_name = target_d
    expression = '100*d^2'
    coupled_variables = 'd'
    outputs = exodus
  []
[]


[Executioner]
  type = Transient

  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist                 '
  automatic_scaling = true

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  dt = 1e-6
  end_time = 4e-6

  fixed_point_max_its = 20
  accept_on_max_fixed_point_iteration = true
  fixed_point_rel_tol = 1e-8
  fixed_point_abs_tol = 1e-10
  fixed_point_min_its = 5
[]

[Outputs]
  print_linear_residuals = false
  file_base = './out/oneElement'
  csv = true

  [exodus]
    type = Exodus
    # execute_on = 'MULTIAPP_FIXED_POINT_END'
  []
[]


[Postprocessors]
  [ave_target_d]
    type = ElementExtremeValue
    variable = 'target_d'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [ave_target_d_avg]
    type = ElementExtremeValue
    variable = 'target_d_avg'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
