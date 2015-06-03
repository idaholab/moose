# MOOSE Test Suite
#-----------------
#
# This test demonstrates the usage of higher order auxiliary variables. A standard Gaussian function is is calculated on
# an eighth order monomial basis on a coarse mesh (4x4). Constant or linear elements would be insufficient to capture
# the curvature of the solution given the number of elements. Oversampling with four levels of refinement is used to visualize
# the higher order solution on finer linear mesh.
#
########################

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -10
  xmax = 10
  ymin = -10
  ymax = 10
  nx = 4
  ny = 4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./parsed]
    # High order monomial basis for capturing high curvature
    order = EIGHTH
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./parsed]
    type = FunctionAux
    # Gaussian distribution centered on the origin
    function = '10*exp(-(pow(x,2)+pow(y,2)))'
    variable = parsed
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 0
  [../]
  [./right_u]
    type = DirichletBC
    variable = u
    boundary = 0
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  output_initial = true
  [./oversample]
    type = Exodus
    refinements = 4
  [../]
[]
