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

[FluidProperties]
  [./fp]
    type = NaNInterfaceTestFluidProperties
  [../]
[]

[Executioner]
  type = Steady
[]
