[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./test_kernel]
    type = NaNInterfaceTestKernel
    variable = u
    nan_interface_test_fp = fp
  [../]
[]

[Modules]
  [./FluidProperties]
    [./fp]
      type = NaNInterfaceTestFluidProperties
      use_quiet_nans = true
    [../]
  []
[]

[Executioner]
  type = Steady
[]
