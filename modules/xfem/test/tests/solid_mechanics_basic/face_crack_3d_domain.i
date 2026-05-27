# Dummy problem that does not solve equilibrium problem
# Cracks driven by presecribed stress and strain fields.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

rad = 0.1
offset = 0
spin = 0
tilt = 0
fname = 'tet_block'

# Elastic constants used by DomainIntegral and by the compatibility strain below.
E = 207000
nu = 0.3

# Prescribed stress field.  The default mirrors the former top_y traction ramp:
#   sigma_yy = sigma0 + sigma_z_slope * z
# Change these constants or sigma_yy_function to prescribe a different field.
sigma0 = 20
sigma_z_slope = 500

[Mesh]
  #---- CUTTER MESH
  [cicle_outline]
    type = ParsedCurveGenerator
    x_formula = '${rad}*cos(t)'
    y_formula = '${rad}*sin(t)'
    section_bounding_t_values = '0 ${fparse 2*pi}'
    nums_segments = '10'
    is_closed_loop = true
  []
  [circle_surface]
    type = XYDelaunayGenerator
    boundary = 'cicle_outline'
    desired_area = 0.001
    output_subdomain_id = 1
  []
  [circle_spin]
    type = TransformGenerator
    input = circle_surface
    transform = ROTATE
    vector_value = '0 0 ${spin}'
  []
  [circle_rotate]
    type = TransformGenerator
    input = circle_spin
    transform = ROTATE
    vector_value = '0 ${fparse 90-tilt} 0'
  []
  [circle_move]
    type = TransformGenerator
    input = circle_rotate
    transform = TRANSLATE
    vector_value = '${offset} 0 -0.01'
    save_with_name = mesh_cutter
    output = true
  []

  #---- FEM MESH
  [FEM_mesh]
    type = FileMeshGenerator
    file = ${fname}.e
  []
  [FEM_mesh_move]
    type = TransformGenerator
    input = FEM_mesh
    transform = TRANSLATE
    vector_value = '0 -0.0001 0'
  []
  [pin]
    type = ExtraNodesetGenerator
    input = FEM_mesh_move
    new_boundary = 'pin'
    coord = '${fparse 2*rad} ${fparse -rad} ${fparse rad}'
    use_closest_node = true
  []
  [center]
    type = ExtraNodesetGenerator
    input = pin
    new_boundary = 'center'
    coord = '0 ${fparse rad} 0'
    use_closest_node = true
  []
  final_generator = center
[]

[Problem]
  solve = false
  kernel_coverage_check = false
  skip_nl_system_check = true
[]

[AuxVariables]
  # dummy displacement for DomainIntegral
  [disp_x]
    initial_condition=0
  []
  [disp_y]
    initial_condition=0
  []
  [disp_z]
    initial_condition=0
  []
[]

[Functions]
  [zero]
    type = ConstantFunction
    value = 0
  []

  # Nonzero prescribed Cauchy stress component.
  [sigma_yy_function]
    type = ParsedFunction
    expression = '${sigma0} + ${sigma_z_slope} * z'
  []

  # A compatible small-strain tensor for DomainIntegral objects that request the
  # elastic_strain material property.  For the default uniaxial stress field this
  # is Hooke's-law compliance: eps_yy = sigma_yy / E,
  # eps_xx = eps_zz = -nu * sigma_yy / E.
  [eps_xx_function]
    type = ParsedFunction
    expression = '-${nu} * (${sigma0} + ${sigma_z_slope} * z) / ${E}'
  []
  [eps_yy_function]
    type = ParsedFunction
    expression = '(${sigma0} + ${sigma_z_slope} * z) / ${E}'
  []
  [eps_zz_function]
    type = ParsedFunction
    expression = '-${nu} * (${sigma0} + ${sigma_z_slope} * z) / ${E}'
  []
[]

[Materials]
  [prescribed_stress]
    type = GenericFunctionRankTwoTensor
    tensor_name = stress
    tensor_functions = '0 0 0 0 sigma_yy_function 0 0 0 0'
  []

  # Provide the elastic_strain material property consumed by InteractionIntegral.
  [prescribed_elastic_strain]
    type = GenericFunctionRankTwoTensor
    tensor_name = elastic_strain
    tensor_functions = 'eps_xx_function 0 0 0 eps_yy_function 0 0 0 eps_zz_function'
  []
  # dummy strain needed by strainEnergyDensity for InteractionIntegral
  [prescribed_mechanical_strain]
    type = GenericConstantRankTwoTensor
    tensor_name = mechanical_strain
    tensor_values = '0 0 0 0 0 0 0 0 0'
  []
[]

[Executioner]
  type = Transient
  start_time = 0.0
  dt = 1.0
  end_time = 4
  max_xfem_update = 1
[]

[Outputs]
[]
