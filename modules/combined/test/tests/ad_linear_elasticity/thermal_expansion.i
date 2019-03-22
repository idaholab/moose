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
  nz = 0
  xmin = 0
  xmax = 2
  ymin = 0
  ymax = 2
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

### Can be used when AD is added to tensor mechanics action
# [Modules/TensorMechanics/Master/All]
#   strain = SMALL
#   eigenstrain_names = eigenstrain
#   add_variables = true
#   generate_output = 'stress_xx stress_yy stress_xy'
# []


### Replaceable by action
[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[Kernels]
  [./x]
    type = ADStressDivergenceTensors
    variable = disp_x
    component = 0
  [../]
  [./y]
    type = ADStressDivergenceTensors
    variable = disp_y
    component = 1
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./stress_xx_aux]
    type = MaterialRankTwoTensorAux
    variable = stress_xx
    i = 0
    j = 0
    property = stress
  [../]
  [./stress_xy_aux]
    type = MaterialRankTwoTensorAux
    variable = stress_xy
    i = 0
    j = 1
    property = stress
  [../]
  [./stress_yy_aux]
    type = MaterialRankTwoTensorAux
    variable = stress_yy
    i = 1
    j = 1
    property = stress
  [../]
[]

[Materials]
  [./strain]
    type = ADComputeSmallStrain
    eigenstrain_names = eigenstrain
  [../]
[]
### Replaceable by action

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric9
    C_ijkl = '1e6 0 0 1e6 0 1e6 .5e6 .5e6 .5e6'
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
    type = PresetBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  [../]
  [./left_x]
    type = PresetBC
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
