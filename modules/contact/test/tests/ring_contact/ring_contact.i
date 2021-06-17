#
# A test of contact with quadratic (Hex20) elements
#
# A stiff ring is pushed into a soft base.  The base shows a circular impression.
#

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = false
[]

[Mesh]
  file = ring_contact.e
[]

[Functions]
  [./ring_y]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 1'
    scale_factor = -0.2
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    strain = FINITE
  [../]
[]

[Contact]
  [./dummy_name]
    primary = 3
    secondary = 2
    penalty = 1e3
    tension_release = -1
  [../]
[]

[BCs]
  [./plane]
    type = DirichletBC
    variable = disp_z
    boundary = 10
    value = 0.0
  [../]

  [./bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]

  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]

  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = 1
    value = 0.0
  [../]

  [./ring_x]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    value = 0.0
  [../]

  [./ring_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 4
    function = ring_y
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./stiffStuff2]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1e3
    poissons_ratio = 0.3
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  [../]
[]  # Materials

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'
  line_search = 'none'
  nl_rel_tol = 1.e-10
  l_max_its = 100
  nl_max_its = 10
  dt = 0.1
  end_time = 0.5

  [./Quadrature]
    order = THIRD
  [../]
[]

[Outputs]
  exodus = true
[]
