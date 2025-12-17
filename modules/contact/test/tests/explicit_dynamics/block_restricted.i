# This input file does not solve any interesting physics, such as contact.
# Its sole purpose is to check that block-restricting the ExplicitDynamicsContact Action
# produces AuxVariables that are appropriately block restricted.
# It illustrates that if your mesh contains blocks that are not involved in the
# contact (block = 3 in the case below) you can restrict the Contact parts
# of your input file (such as the ExplicitDynamicsContact Action, the
# velocity AuxVariable, the WaveSpeed Material) to exist only on the
# parts of mesh involved in contact (blocks 1 and 2 in the case below)
[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1'
    ix = '1 1 1'
    dy = 1
    iy = 1
    subdomain_id = '1 2 3'
  []
  [inner_bdy]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    new_boundary = inner
    primary_block = 2
    paired_block = 1
  []
[]

[ExplicitDynamicsContact]
  [roof_floor]
    model = frictionless
    block = '1 2'
    primary = inner
    secondary = left
    vel_x = vel_x
    vel_y = vel_y
    penalty = 1
    verbose = true
  []
[]

[AuxVariables]
  [vel_x]
    block = '1 2'
  []
  [vel_y]
    block = '1 2'
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
[]

[Materials]
  [wave_speed]
    type = WaveSpeed
    implicit = false
    block = '1 2'
  []
[]

# From here on, nothing is particularly interesting!
# I have included block = '1 2 3' explicitly, to distinguish from the above

[Problem]
  extra_tag_matrices = mass
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [disp_x]
    block = '1 2 3'
  []
  [disp_y]
    block = '1 2 3'
  []
[]

[Kernels]
  [solid_x]
    type = DynamicStressDivergenceTensors
    component = 0
    variable = disp_x
    implicit = false
    block = '1 2 3'
  []
  [solid_y]
    type = DynamicStressDivergenceTensors
    component = 1
    variable = disp_y
    implicit = false
    block = '1 2 3'
  []
  [massmatrix_x]
    type = MassMatrix
    density = density
    matrix_tags = mass
    variable = disp_x
    block = '1 2 3'
  []
  [massmatrix_y]
    type = MassMatrix
    density = density
    matrix_tags = mass
    variable = disp_y
    block = '1 2 3'
  []
[]

[BCs]
  [no_x]
    type = ExplicitDirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
    implicit = false
  []
  [no_y]
    type = ExplicitDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
    implicit = false
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0
    implicit = false
    block = '1 2 3'
  []
  [strain]
    type = ComputeSmallStrain
    implicit = false
    block = '1 2 3'
  []
  [stress]
    type = ComputeLinearElasticStress
    implicit = false
    block = '1 2 3'
  []
  [density]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 1
    implicit = false
    block = '1 2 3'
  []
[]

[Executioner]
  type = Transient
  [TimeIntegrator]
    type = ExplicitMixedOrder
    mass_matrix_tag = mass
    second_order_vars = 'disp_x disp_y'
  []
  dt = 1
  end_time = 1
[]

[Outputs]
  exodus = true
  console = true
[]
