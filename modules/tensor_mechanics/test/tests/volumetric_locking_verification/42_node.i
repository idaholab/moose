# Test for volumetric locking correction

# 2D cook's membrane problem with a trapezoid
# that is fixed at one end and is sheared at
# other end. Poisson's ratio is 0.4999.

# Using Quad4 elements and no volumetric locking,
# vertical displacement at top right corner is 3.78
# due to locking.

# Using Quad4 elements with volumetric locking, vertical
# dispalcement at top right corner is 7.78.

# Results match with Nakshatrala et al., Comp. Mech., 41, 2008.

# Check volumetric locking correction documentation for
# more details about this problem.

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  file = 42_node_side.e
[]

[Modules]
  [./TensorMechanics]
    [./Master]
      [./all]
        add_variables = true
        strain = SMALL
        incremental = true
      [../]
    [../]
  [../]
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
[]

[NodalKernels]
  [./y_force]
    type = ConstantRate
    variable = disp_y
    boundary = 2
    rate = 2.38095238095
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 250.0
    poissons_ratio = 0.4999
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]


[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  num_steps = 1
[]


[Postprocessors]
  [./a_disp_y]
    type = PointValue
    variable = disp_y
    point = '48.0 60.0 0.0'
  [../]
[]

[Outputs]
  exodus = true
[]
