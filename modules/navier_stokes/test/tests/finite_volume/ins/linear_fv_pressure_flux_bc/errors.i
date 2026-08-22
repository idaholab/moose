[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
  []
[]

[Problem]
  linear_sys_names = pressure_system
[]

[Variables]
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = Ainv
  []
[]

[LinearFVBCs]
  active = pressure_flux
  [pressure_flux]
    type = LinearFVPressureFluxBC
    variable = pressure
    boundary = left
    HbyA_flux = 0
    Ainv = Ainv
    u = 0
    rho = 1
  []
[]

[FunctorMaterials]
  [inverse_diagonal]
    type = GenericVectorFunctorMaterial
    prop_names = Ainv
    prop_values = '1 1 1'
  []
[]

[Executioner]
  type = Steady
  system_names = pressure_system
[]
