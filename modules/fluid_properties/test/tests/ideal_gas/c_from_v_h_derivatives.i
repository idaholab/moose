# Tests the derivatives of the c(v,h) call

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [./v]
    initial_condition = 2.0
  [../]
  [./h]
    initial_condition = 3.0
  [../]
[]

[Modules]
  [./FluidProperties]
    [./fp]
      type = IdealGasFluidProperties
      gamma = 1.4
      R = 1.0
      mu = 4.0
      k = 3.0
    [../]
  []
[]

[Kernels]
  [./test_kernel]
    type = SoundSpeedFromVolumeEnthalpyDerivativesTestKernel
    variable = v
    fluid_properties = fp
    v = v
    h = h
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
