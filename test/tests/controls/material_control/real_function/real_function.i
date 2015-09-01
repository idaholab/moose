[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./test]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./aux]
    type = MaterialRealAux
    variable = test
    property = 'test_prop'
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Materials]
  [./mat]
    type = GenericConstantMaterial
    prop_values = '98'
    prop_names = 'test_prop'
    block = 0
  [../]
[]

[Functions]
  [./func]
    type = ParsedFunction
    value = sin(2*pi*x)*sin(2*pi*y)*t
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  output_initial = true
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[]

[Controls]
  [./mat_func]
    type = RealFunctionControlMaterial
    function = func
    property = 'test_prop'
    block = 0
  [../]
[]
