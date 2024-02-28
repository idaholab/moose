[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  nz = 0
  xmax = 3
  ymax = 2
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./dummy]
  [../]
[]

[AuxVariables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = sin(x)*0.1
    [../]
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = cos(y)*0.05
    [../]
  [../]
  [./E]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./elastic_energy]
    type = ElasticEnergyAux
    variable = E
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeElasticityTensor
    fill_method = symmetric9
    C_ijkl = '1 2 4 3 2 5 1 3 1'
  [../]
  [./strain]
    type = ComputeSmallStrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Problem]
  kernel_coverage_check = false
[]

[Outputs]
  exodus = true
[]
