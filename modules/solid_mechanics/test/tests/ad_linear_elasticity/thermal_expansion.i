# This input file is designed to test the RankTwoAux and RankFourAux
# auxkernels, which report values out of the Tensors used in materials
# properties.
# Materials properties into AuxVariables - these are elemental variables, not nodal variables.

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmax = 2
  ymax = 2
[]

[Modules/TensorMechanics/Master/All]
  strain = SMALL
  eigenstrain_names = eigenstrain
  add_variables = true
  generate_output = 'stress_xx stress_yy stress_xy'
  use_automatic_differentiation = true
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0
  [../]
  [./stress]
    type = ADComputeLinearElasticStress
  [../]
  [./eigenstrain]
    type = ADComputeEigenstrain
    eigen_base = '1e-4'
    eigenstrain_name = eigenstrain
  [../]
[]

[BCs]
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  [../]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'

  nl_rel_tol = 1e-14
[]

[Outputs]
  exodus = true
[]
