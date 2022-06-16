# Test for `StructureAcousticInterface` interface kernel. The domain is 1D with 20m
# length. The fluid domain is on the right and the structural domain is on the left.
# Fluid end is subjected to a 250Hz sine wave with a single peak of amplitude unity.
# Structural domain is 4 times as dense as the fluid domain with all other material
# properties being the same. Fluid pressure is recorded at the midpoint in the fluid
# domain (i.e., at 15m). Structural stress is recorded at the midpoint in the structural
# domain (i.e., at 5m). The recorded pressure and stress amplitudes should match
# with theoretical values.
#
# Input parameters:
# Dimensions = 1
# Length = 20 meters
# Fluid speed of sound = 1500 m/s
# Fluid density = 1e-6 Giga kg/m^3
# Structural bulk modulus = 2.25 GPa
# Structural shear modulus = 0 GPa
# Structural density = 4e-6 Giga kg/m^3
# Fluid domain = true
# Fluid BC = single peak sine wave applied as a pressure on the fluid end
# Structural domain = true
# Structural BC = Neumann BC with value zero applied on the structural end.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 50
    xmax = 20
  []
  [./subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '10.0 0 0'
    block_id = 1
    top_right = '20.0 0.0 0'
  [../]
  [./interface1]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain1
    primary_block = '1'
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
  [./stress_xx]
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
    displacements = 'disp_x'
    block = 0
  [../]
  [./inertia_x1]
    type = InertialForce
    variable = disp_x
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
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
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
[]

[Functions]
  [./accel_bottom]
    type = PiecewiseLinear
    data_file = Input_1Peak_highF.csv
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
    prop_values = 4e-6
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
    displacements = 'disp_x'
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
  end_time = 0.01
  dt = 0.0001
  dtmin = 0.00001
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-12
  l_tol = 1e-12
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
    point = '10.0 0.0 0.0'
    variable = p
  [../]
  [./stress1]
    type = PointValue
    point = '10.0 0.0 0.0'
    variable = stress_xx
  [../]
[]

[Outputs]
  csv = true
  perf_graph = true
  print_linear_residuals = true
[]
