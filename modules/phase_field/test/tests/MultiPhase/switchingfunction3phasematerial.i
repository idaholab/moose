# This is a test of the SwitchingFunction3PhaseMaterial, a switching function
# used in a 3-phase phase-field model to prevent formation of the third phase
# at the interface between the two other phases
# See Folch and Plapp, Phys. Rev. E, v 72, 011602 (2005) for details

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 0.1
  nx = 20
  ny = 2
  elem_type = QUAD4
[]

[GlobalParams]
  derivative_order = 0
  outputs = exodus
[]

[AuxVariables]
  [./eta1]
    [./InitialCondition]
      type = FunctionIC
      function = x
    [../]
  [../]
  [./eta2]
    [./InitialCondition]
      type = FunctionIC
      function = 1.0-x
    [../]
  [../]
  [./eta3]
    [./InitialCondition]
      type = ConstantIC
      value = 0.0
    [../]
  [../]
[]

[Materials]
  [./h_material_1]
    type = SwitchingFunction3PhaseMaterial
    property_name = h_i1
    eta_i = eta1
    eta_j = eta2
    eta_k = eta3
    outputs = exodus
  [../]
# Next we reverse eta2 and eta3 to make sure the switching function is symmetric
# with respect to interchanging these two, as it is designed to be
  [./h_material_2]
    type = SwitchingFunction3PhaseMaterial
    property_name = h_i2
    eta_i = eta1
    eta_j = eta3
    eta_k = eta2
    outputs = exodus
  [../]
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  exodus = true
[]
