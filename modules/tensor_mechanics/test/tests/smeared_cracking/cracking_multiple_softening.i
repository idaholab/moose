# Test of cracking with direction-specific release models in 3
# directions. Block is first pulled in one direction, and then
# held while it is sequentially pulled in the other two
# directions. Poisson's ratio is zero so that the cracking in one
# direction doesn't affect the others.

# Softening in the three directions should follow the laws for the
# prescribed models in the three directions, which are power law (x),
# exponential (y), and abrupt (z).

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
  [./displx]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '0 1 1 1'
  [../]
  [./disply]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '0 0 1 1'
  [../]
  [./displz]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '0 0 0 1'
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
  [../]
[]

[BCs]
  [./pullx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = displx
  [../]
  [./pully]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = disply
  [../]
  [./pullz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = displz
  [../]
  [./left]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./bottom]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./back]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.8e7
    poissons_ratio = 0
  [../]
  [./elastic_stress]
    type = ComputeSmearedCrackingStress
    cracking_stress = 1.68e6
    softening_models = 'power_law_softening exponential_softening abrupt_softening'
    prescribed_crack_directions = 'x y z'
  [../]
  [./power_law_softening]
    type = PowerLawSoftening
    stiffness_reduction = 0.3333
  [../]
  [./exponential_softening]
    type = ExponentialSoftening
  [../]
  [./abrupt_softening]
    type = AbruptSoftening
  [../]
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-ksp_gmres_restart -pc_type -sub_pc_type'
  petsc_options_value = '101                asm      lu'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  l_tol = 1e-5
  start_time = 0.0
  end_time = 3.0
  dt = 0.01
[]

[Outputs]
  exodus = true
[]
