# Test for `StructureAcousticInterface` interface kernel. The domain is 3D with lengths
# 10 X 0.1 X 0.1 meters. The fluid domain is on the right and the structural domain
# is on the left. Fluid end is subjected to a 250Hz sine wave with a single peak.
# Structural domain has the same material properties as the fluid. Interface between
# structure and fluid is located at 5.0m in the x-direction. Fluid pressure is recorded
# at (5, 0.05, 0.05). Structural stress is also recorded at the same location. Fluid
# pressure and structural stress should be almost equal and opposite to each other.
#
# Input parameters:
# Dimensions = 3
# Lengths = 10 X 0.1 X 0.1 meters
# Fluid speed of sound = 1500 m/s
# Fluid density = 1e-6 Giga kg/m^3
# Structural bulk modulus = 2.25 GPa
# Structural shear modulus = 0 GPa
# Structural density = 1e-6 Giga kg/m^3
# Fluid domain = true
# Fluid BC = single peak sine wave applied as a pressure on the fluid end
# Structural domain = true
# Structural BC = Neumann BC with value zero applied on the structural end.
# Fluid-structure interface location = 5.0m along the x-direction

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 100
    ny = 1
    nz = 1
    xmax = 10
    ymax = 0.1
    zmax = 0.1
  []
  [./subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '5.0 0.0 0.0'
    block_id = 1
    top_right = '10.0 0.1 0.1'
  [../]
  [./interface1]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain1
    primary_block = 1
    paired_block = 0
    new_boundary = 'interface1'
  [../]
[]

[GlobalParams]
[]

[Variables]
  [./p]
    block = 1
  [../]
  [./disp_x]
    block = 0
  [../]
  [./disp_y]
    block = 0
  [../]
  [./disp_z]
    block = 0
  [../]
[]

[AuxVariables]
  [./vel_x]
    order = FIRST
    family = LAGRANGE
    block = 0
  [../]
  [./accel_x]
    order = FIRST
    family = LAGRANGE
    block = 0
  [../]
  [./vel_y]
    order = FIRST
    family = LAGRANGE
    block = 0
  [../]
  [./accel_y]
    order = FIRST
    family = LAGRANGE
    block = 0
  [../]
  [./vel_z]
    order = FIRST
    family = LAGRANGE
    block = 0
  [../]
  [./accel_z]
    order = FIRST
    family = LAGRANGE
    block = 0
  [../]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
  [./stress_xz]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
[]

[Kernels]
  [./diffusion]
    type = Diffusion
    variable = 'p'
    block = 1
  [../]
  [./inertia]
    type = AcousticInertia
    variable = p
    block = 1
  [../]
  [./DynamicTensorMechanics]
    displacements = 'disp_x disp_y disp_z'
    block = 0
  [../]
  [./inertia_x]
    type = InertialForce
    variable = disp_x
    block = 0
  [../]
  [./inertia_y]
    type = InertialForce
    variable = disp_y
    block = 0
  [../]
  [./inertia_z]
    type = InertialForce
    variable = disp_z
    block = 0
  [../]
[]

[AuxKernels]
  [./accel_x]
    type = TestNewmarkTI
    displacement = disp_x
    variable = accel_x
    first = false
    block = 0
  [../]
  [./vel_x]
    type = TestNewmarkTI
    displacement = disp_x
    variable = vel_x
    block = 0
  [../]
  [./accel_y]
    type = TestNewmarkTI
    displacement = disp_y
    variable = accel_y
    first = false
    block = 0
  [../]
  [./vel_y]
    type = TestNewmarkTI
    displacement = disp_y
    variable = vel_y
    block = 0
  [../]
  [./accel_z]
    type = TestNewmarkTI
    displacement = disp_z
    variable = accel_z
    first = false
    block = 0
  [../]
  [./vel_z]
    type = TestNewmarkTI
    displacement = disp_z
    variable = vel_z
    block = 0
  [../]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    block = 0
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    block = 0
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
    block = 0
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
    block = 0
  [../]
  [./stress_xz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xz
    index_i = 0
    index_j = 2
    block = 0
  [../]
  [./stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yz
    index_i = 1
    index_j = 2
    block = 0
  [../]
[]

[InterfaceKernels]
  [./interface1]
    type =  StructureAcousticInterface
    variable = p
    neighbor_var = disp_x
    boundary = 'interface1'
    D = 1e-6
    component = 0
  [../]
  [./interface2]
    type =  StructureAcousticInterface
    variable = p
    neighbor_var = disp_y
    boundary = 'interface1'
    D = 1e-6
    component = 1
  [../]
  [./interface3]
    type =  StructureAcousticInterface
    variable = p
    neighbor_var = disp_z
    boundary = 'interface1'
    D = 1e-6
    component = 2
  [../]
[]

[BCs]
  [./bottom_accel]
    type = FunctionDirichletBC
    variable = p
    boundary = 'right'
    function = accel_bottom
  [../]
  [./disp_x1]
    type = NeumannBC
    boundary = 'left'
    variable = disp_x
    value = 0.0
  [../]
  [./disp_y1]
    type = NeumannBC
    boundary = 'left'
    variable = disp_y
    value = 0.0
  [../]
  [./disp_z1]
    type = NeumannBC
    boundary = 'left'
    variable = disp_z
    value = 0.0
  [../]
[]

[Functions]
  [./accel_bottom]
    type = PiecewiseLinear
    data_file = ../1D_struc_acoustic/Input_1Peak_highF.csv
    scale_factor = 1e-2
    format = 'columns'
  [../]
[]

[Materials]
  [./co_sq]
    type = GenericConstantMaterial
    prop_names = inv_co_sq
    prop_values = 4.44e-7
    block = '1'
  [../]
  [./density0]
    type = GenericConstantMaterial
    block = 0
    prop_names = density
    prop_values = 1e-6
  [../]
  [./elasticity_base]
    type = ComputeIsotropicElasticityTensor
    bulk_modulus = 2.25
    shear_modulus = 0.0
    block = 0
  [../]
  [./strain]
    type = ComputeFiniteStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    type =  ComputeFiniteStrainElasticStress
    block = 0
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  start_time = 0.0
  end_time = 0.005
  dt = 0.0001
  dtmin = 0.00001
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  l_tol = 1e-8
  l_max_its = 25
  timestep_tolerance = 1e-8
  automatic_scaling = true
  [TimeIntegrator]
    type = NewmarkBeta
  []
[]

[Postprocessors]
  [./p1]
    type = PointValue
    point = '5.0 0.05 0.05'
    variable = p
  [../]
  [./stress_xx]
    type = PointValue
    point = '5.0 0.05 0.05'
    variable = stress_xx
  [../]
[]

[Outputs]
  csv = true
  perf_graph = true
  print_linear_residuals = true
[]
