# Tests the derivatives of the s(h,p) call

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [./h]
    initial_condition = 2.0
  [../]
  [./p]
    initial_condition = 3.0
  [../]
[]

[Modules]
  [./FluidProperties]
    [./fp]
      type = StiffenedGasFluidProperties
      gamma = 1.5
      q = -10.0
      q_prime = 5.0
      p_inf = 2.0
      cv = 2.0
      mu = 4.0
      k = 3.0
    [../]
  []
[]

[Kernels]
  [./test_kernel]
    type = EntropyFromEnthalpyPressureDerivativesTestKernel
    variable = h
    fluid_properties = fp
    h = h
    p = p
  [../]
[]

[Problem]
  kernel_coverage_check = false
[]

[Preconditioning]
  [./preconditioner]
    type = SMP
    full = true
    solve_type = NEWTON
    petsc_options_iname = '-snes_type -snes_test_err'
    petsc_options_value = 'test       1e-8'
  [../]
[]

[Executioner]
  type = Steady
[]
