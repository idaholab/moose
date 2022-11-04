###########################################################
# This is a test of the material derivatives test kernel.
###########################################################

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./u_IC_fn]
    type = ParsedFunction
    expression = 'x'
  [../]
  [./v_IC_fn]
    type = ParsedFunction
    expression = 'sin(x)'
  [../]
[]

[ICs]
  [./u_IC]
    type = FunctionIC
    variable = u
    function = u_IC_fn
  [../]
  [./v_IC]
    type = FunctionIC
    variable = v
    function = v_IC_fn
  [../]
[]

[Kernels]
  [./test_kernel]
    type = MaterialDerivativeTestKernel
    variable = u
    coupled_variables = 'u v'
    material_property = material_derivative_test_property
  [../]
  # add a dummy kernel for v to prevent singular Jacobian
  [./dummy_kernel]
    type = Diffusion
    variable = v
  [../]
[]

[Materials]
  [./material_derivative_test_material]
    type = MaterialDerivativeTestMaterial
    var1 = u
    var2 = v
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
    solve_type = newton
    petsc_options_iname = '-snes_type -snes_test_err'
    petsc_options_value = 'test       1e-10'
  [../]
[]

[Executioner]
  type = Steady
[]
