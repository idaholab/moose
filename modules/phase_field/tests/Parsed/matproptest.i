#
# This test validates the helper materials that generate material properties for
# the h(eta) switching function and the g(eta) double well function
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 5
  nz = 0
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  # order parameter 1
  [./eta1]
    order = FIRST
    family = LAGRANGE
  [../]
  [./eta2]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Materials]
  [./h_eta1]
    type = SwitchingFunctionMaterial
    block = 0
    function_name = h1
    h_order = SIMPLE
    eta = eta1
  [../]
  [./h_eta2]
    type = SwitchingFunctionMaterial
    block = 0
    function_name = h2
    h_order = SIMPLE
    eta = eta2
  [../]
  [./penalty]
    type = DerivativeParsedMaterial
    block = 0
    function = '(1-h1-h2)^2'
    material_property_names = 'h1(eta1) h2(eta1,eta2)'
    outputs = exodus
  [../]
[]

[Kernels]
  [./eta1diff]
    type = Diffusion
    variable = eta1
  [../]
  [./eta2diff]
    type = Diffusion
    variable = eta2
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  print_linear_residuals = true
[]
