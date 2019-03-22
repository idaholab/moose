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

# [Modules/TensorMechanics/Master/All]
#   strain = SMALL
#   eigenstrain_names = eigenstrain
#   add_variables = true
#   generate_output = 'strain_xx strain_yy strain_xy'
# []

### Everything between here and the next ### should be replaceable by future action availability
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
  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./strain_xx_aux]
    type = MaterialRankTwoTensorAux
    variable = strain_xx
    i = 0
    j = 0
    property = total_strain
  [../]
  [./strain_yy_aux]
    type = MaterialRankTwoTensorAux
    variable = strain_yy
    i = 1
    j = 1
    property = total_strain
  [../]
  [./strain_xy_aux]
    type = MaterialRankTwoTensorAux
    variable = strain_xy
    i = 0
    j = 1
    property = total_strain
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
    eigen_base = '0.1 0.05 0 0 0 0.01'
    prefactor = -1
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
[]

[Outputs]
  exodus = true
[]
