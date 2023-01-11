[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
  xmax = 1
  ymax = 1
  zmax = 1
  xmin = -1
  ymin = -1
  zmin = -1
[]

[GlobalParams]
  order = CONSTANT
  family = MONOMIAL
  rank_two_tensor = extra_stress
[]

[Functions]
  [./sphere1]
    type = ParsedFunction
    expression = 'r:=sqrt(x^2+y^2+z^2); if(r>1,0,1-3*r^2+2*r^3)'
  [../]
  [./sphere2]
    type = ParsedFunction
    expression = 'r:=sqrt(x^2+y^2+z^2); 0.5-0.5*if(r>1,0,1-3*r^2+2*r^3)'
  [../]
[]

[Variables]
  [./dummy]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = dummy
  [../]
[]

[AuxVariables]
  [./eta1]
    [./InitialCondition]
      type = FunctionIC
      function = sphere1
    [../]
    order = FIRST
    family = LAGRANGE
  [../]
  [./eta2]
    [./InitialCondition]
      type = FunctionIC
      function = sphere2
    [../]
    order = FIRST
    family = LAGRANGE
  [../]
  [./s00]
  [../]
  [./s01]
  [../]
  [./s02]
  [../]
  [./s10]
  [../]
  [./s11]
  [../]
  [./s12]
  [../]
  [./s20]
  [../]
  [./s21]
  [../]
  [./s22]
  [../]
[]

[AuxKernels]
  [./s00]
    type = RankTwoAux
    variable = s00
    index_i = 0
    index_j = 0
  [../]
  [./s01]
    type = RankTwoAux
    variable = s01
    index_i = 0
    index_j = 1
  [../]
  [./s02]
    type = RankTwoAux
    variable = s02
    index_i = 0
    index_j = 2
  [../]
  [./s10]
    type = RankTwoAux
    variable = s10
    index_i = 1
    index_j = 0
  [../]
  [./s11]
    type = RankTwoAux
    variable = s11
    index_i = 1
    index_j = 1
  [../]
  [./s12]
    type = RankTwoAux
    variable = s12
    index_i = 1
    index_j = 2
  [../]
  [./s20]
    type = RankTwoAux
    variable = s20
    index_i = 2
    index_j = 0
  [../]
  [./s21]
    type = RankTwoAux
    variable = s21
    index_i = 2
    index_j = 1
  [../]
  [./s22]
    type = RankTwoAux
    variable = s22
    index_i = 2
    index_j = 2
  [../]
[]

[Materials]
  [./interface]
    type = ComputeInterfaceStress
    v = 'eta1 eta2'
    stress   = '1.0 2.0'
    op_range = '1.0 0.5'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  file_base = test_out
  execute_on = timestep_end
  hide = 'dummy eta1 eta2'
[]
