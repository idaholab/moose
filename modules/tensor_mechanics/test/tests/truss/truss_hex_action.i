# This test is designed to check
# whether truss element works well with other multi-dimensional element
# e.g. the hex element in this case, by assigning different block number
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
  [./TensorMechanics]
    block = 1000
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[Modules/TensorMechanics/LineElementMaster]
   [./block]
     truss = true
     displacements = 'disp_x disp_y disp_z'

     area = area

     block = '1 2'
     save_in = 'react_x react_y react_z'
   [../]
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
  file_base = 'truss_hex_out'
  exodus = true
[]
