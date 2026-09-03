# Almost identical to moving_block, but with mesh refinement
# Illustrating that although a Marker is defined on block = 0,
# because restrict_to_active_blocks = true
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  block = '1 2'
[]

[Problem]
  extra_tag_matrices = 'mass'
#  material_coverage_check = SKIP_LIST
#  material_coverage_block_list = 0
[]

[Mesh]
  [mesh3D]
    type = CartesianMeshGenerator
    dim = 3
    dx = '1 0.5 1'
    ix = '1 1 1'
    dy = 1
    iy = 1
    dz = 1
    iz = 1
    subdomain_id = '1 0 2'
  []
  [floor]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh3D
    new_boundary = floor
    primary_block = 1
    paired_block = 0
  []
  [roof]
    type = SideSetsBetweenSubdomainsGenerator
    input = floor
    new_boundary = roof
    primary_block = 2
    paired_block = 0
  []
[]

[ExplicitDynamicsContact]
  [roof_floor]
    model = frictionless
    primary = floor
    secondary = roof
    tangential_tolerance = 1
    vel_x = vel_x
    vel_y = vel_y
    vel_z = vel_z
    penalty = 1E-2 # sufficiently small that interpenetration occurs
    verbose = false
  []
[]
[AuxVariables]
  [vel_x]
  []
  [vel_y]
  []
  [vel_z]
  []
[]
[AuxKernels]
  [vel_x]
    type = TestNewmarkTI
    variable = vel_x
    displacement = disp_x
    execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END'
  []
  [vel_y]
    type = TestNewmarkTI
    variable = vel_y
    displacement = disp_y
    execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END'
  []
  [vel_z]
    type = TestNewmarkTI
    variable = vel_z
    displacement = disp_z
    execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END'
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Kernels]
  [solid_x]
    type = DynamicStressDivergenceTensors
    component = 0
    variable = disp_x
    alpha = 0
    use_displaced_mesh = false
    zeta = 0.1
    implicit = false
  []
  [solid_y]
    type = DynamicStressDivergenceTensors
    component = 1
    variable = disp_y
    alpha = 0
    use_displaced_mesh = false
    zeta = 0.1
    implicit = false
  []
  [solid_z]
    type = DynamicStressDivergenceTensors
    component = 2
    variable = disp_z
    alpha = 0
    use_displaced_mesh = false
    zeta = 0.1
    implicit = false
  []
  [massmatrix_x]
    type = MassMatrix
    density = density
    matrix_tags = 'mass'
    variable = disp_x
  []
  [massmatrix_y]
    type = MassMatrix
    density = density
    matrix_tags = 'mass'
    variable = disp_y
  []
  [massmatrix_z]
    type = MassMatrix
    density = density
    matrix_tags = 'mass'
    variable = disp_z
  []
[]
[NodalKernels]
  [damping_x]
    type = ExplicitMassDamping
    variable = disp_x
    eta = 1
    implicit = false
  []
  [damping_y]
    type = ExplicitMassDamping
    variable = disp_y
    eta = 1
    implicit = false
  []
  [damping_z]
    type = ExplicitMassDamping
    variable = disp_z
    eta = 1
    implicit = false
  []
[]

[BCs]
  [no_z]
    type = ExplicitDirichletBC
    variable = disp_z
    boundary = 'left right floor roof'
    value = 0.0
    implicit = false
  []
  [no_y]
    type = ExplicitDirichletBC
    variable = disp_y
    boundary = 'left right floor roof'
    value = 0.0
    implicit = false
  []
  [no_x]
    type = ExplicitDirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
    implicit = false
  []
  [move_x]
    type = ExplicitDirichletBC
    variable = disp_x
    boundary = right
    value = -0.1
  []
[]

[Materials]
  [elasticity123]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0
    implicit = false
  []
  [strain]
    type = ComputeSmallStrain
    implicit = false
  []
  [stress]
    type = ComputeLinearElasticStress
    implicit = false
  []
  [density]
    type = GenericConstantMaterial
    implicit = false
    prop_names = density
    prop_values = 1
  []
[]

[Adaptivity]
  [Markers]
    [uniform]
      type = UniformMarker
      mark = REFINE
# even though the marker is defined on block = 0, refinement does not occur there due to restrict_to_active_blocks = true
      block = '0 1 2'
    []
  []
  marker = uniform
  max_h_level = 1
  interval = 2
[]

[Executioner]
  type = Transient
  [TimeIntegrator]
    type = ExplicitMixedOrder
    mass_matrix_tag = 'mass'
    second_order_vars = 'disp_x disp_y disp_z'
    use_constant_mass = true
    recompute_mass_matrix_after_mesh_change = true
    restrict_to_active_blocks = true
  []
  dt = 0.5
  dtmin = 0.5
  end_time = 2
[]

[Outputs]
  exodus = true
  console = true
[]
