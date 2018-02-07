# Tests the derivatives of the c(v,e) call

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [./v]
    initial_condition = 2.0
  [../]
  [./e]
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
    type = SoundSpeedFromVolumeInternalEnergyDerivativesTestKernel
    variable = v
    fluid_properties = fp
    v = v
    e = e
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
