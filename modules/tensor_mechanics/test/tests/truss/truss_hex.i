# This test is designed to check
# whether truss element works well with other multi-dimensional element
# e.g. the hex element in this case, by assigning different brock number
# to different types of elements.
[Mesh]
  type = FileMesh
  file = truss_hex.e
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./axial_stress]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e_over_l]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./area]
    order = CONSTANT
    family = MONOMIAL
#    initial_condition = 1.0
  [../]
  [./react_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./react_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./react_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./x2]
    type = PiecewiseLinear
    x = '0  1 2 3'
    y = '0 .5 1 1'
  [../]
  [./y2]
    type = PiecewiseLinear
    x = '0 1  2 3'
    y = '0 0 .5 1'
  [../]
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = 1
    value = 0
  [../]

  [./fixx2]
    type = DirichletBC
    variable = disp_x
    boundary = 2
    value = 0
  [../]
  [./fixz2]
    type = DirichletBC
    variable = disp_z
    boundary = 2
    value = 0
  [../]

  [./fixDummyHex_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1000
    value = 0
  [../]

  [./fixDummyHex_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1000
    value = 0
  [../]

  [./fixDummyHex_z]
    type = DirichletBC
    variable = disp_z
    boundary = 1000
    value = 0
  [../]
[]

[DiracKernels]
  [./pull]
    type = ConstantPointSource
    value = -25
    point = '0 -2 0'
    variable = disp_y
  [../]
[]

[AuxKernels]
  [./axial_stress]
    type = MaterialRealAux
    block = '1 2'
    property = axial_stress
    variable = axial_stress
  [../]
  [./e_over_l]
    type = MaterialRealAux
    block = '1 2'
    property = e_over_l
    variable = e_over_l
  [../]
  [./area1]
    type = ConstantAux
    block = 1
    variable = area
    value = 1.0
    execute_on = 'initial timestep_begin'
  [../]
  [./area2]
    type = ConstantAux
    block = 2
    variable = area
    value = 0.25
    execute_on = 'initial timestep_begin'
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

  solve_type = PJFNK

  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'jacobi   101'

  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  dt = 1
  num_steps = 1
  end_time = 1
[]

[Kernels]
  [./truss_x]
    type = StressDivergenceTensorsTruss
    block = '1 2'
    variable = disp_x
    displacements = 'disp_x disp_y disp_z'
    component = 0
    area = area
    save_in = react_x
  [../]
  [./truss_y]
    type = StressDivergenceTensorsTruss
    block = '1 2'
    variable = disp_y
    component = 1
    displacements = 'disp_x disp_y disp_z'
    area = area
    save_in = react_y
  [../]
  [./truss_z]
    type = StressDivergenceTensorsTruss
    block = '1 2'
    variable = disp_z
    component = 2
    displacements = 'disp_x disp_y disp_z'
    area = area
    save_in = react_z
  [../]
  [./TensorMechanics]
    block = 1000
    displacements = 'disp_x disp_y disp_z'
  [../]
#  [./hex_x]
#    type = StressDivergenceTensors
#    block = 1000
#    variable = disp_x
#    component = 0
#    displacements = 'disp_x disp_y disp_z'
#  [../]
#  [./hex_y]
#    type = StressDivergenceTensors
#    block = 1000
#    variable = disp_y
#    component = 1
#    displacements = 'disp_x disp_y disp_z'
#  [../]
#  [./hex_z]
#    type = StressDivergenceTensors
#    block = 1000
#    variable = disp_z
#    component = 2
#    displacements = 'disp_x disp_y disp_z'
#  [../]
[]

[Materials]
   [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 1000
    youngs_modulus = 1e6
    poissons_ratio = 0
  [../]
  [./strain]
    type = ComputeSmallStrain
    block = 1000
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = 1000
  [../]
  [./linelast]
    type = LinearElasticTruss
    block = '1 2'
    displacements = 'disp_x disp_y disp_z'
    youngs_modulus = 1e6
  [../]
[]

[Outputs]
  exodus = true
[]
