[Mesh]
  type = MFEMMesh
  file = ../mesh/ermes_mouse_coarse.g
  dim = 3
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []
  [HDivFESpace]
    type = MFEMVectorFESpace
    fec_type = RT
    fec_order = CONSTANT
  []
[]

[Variables]
  [E]
    type = MFEMComplexVariable
    fespace = HCurlFESpace
  []
[]

[Functions]
  [PI]
    type = ParsedFunction
    expression = 3.1415926535897932384
  []
  [epsilon0]
    type = ParsedFunction
    expression = 8.8541878176e-12
  []
  [mu0]
    type = ParsedFunction
    expression = PI*4e-7
    symbol_names = 'PI'
    symbol_values = 'PI'
  []
  [magnetic_reluctivity]
    type = ParsedFunction
    expression = 1/mu0
    symbol_names = mu0
    symbol_values = mu0
  []
  [freq]
    type = ParsedFunction
    expression = 9e8
  []
  [angfreq]
    type = ParsedFunction
    expression = 2*PI*freq
    symbol_names = 'freq PI'
    symbol_values = 'freq PI'
  []
  [elec_cond_mouse]
    type = ParsedFunction
    expression = 0.0
  []
  [dielec_perm_mouse]
    type = ParsedFunction
    expression = epsilon0
    symbol_names = epsilon0
    symbol_values = epsilon0
  []
  [mass_coef_mouse]
    type = ParsedFunction
    expression = -dielec_perm_mouse*angfreq^2
    symbol_names = 'dielec_perm_mouse angfreq'
    symbol_values = 'dielec_perm_mouse angfreq'
  []
  [loss_coef_mouse]
    type = ParsedFunction
    expression = angfreq*elec_cond_mouse
    symbol_names = 'elec_cond_mouse angfreq'
    symbol_values = 'elec_cond_mouse angfreq'
  []
  [elec_cond_air]
    type = ParsedFunction
    expression = 0.97
  []
  [dielec_perm_air]
    type = ParsedFunction
    expression = 43*epsilon0
    symbol_names = epsilon0
    symbol_values = epsilon0
  []
  [mass_coef_air]
    type = ParsedFunction
    expression = -dielec_perm_air*angfreq^2
    symbol_names = 'dielec_perm_air angfreq'
    symbol_values = 'dielec_perm_air angfreq'
  []
  [loss_coef_air]
    type = ParsedFunction
    expression = angfreq*elec_cond_air
    symbol_names = 'elec_cond_air angfreq'
    symbol_values = 'elec_cond_air angfreq'
  []
  [vecZero]
    type = ParsedVectorFunction
    expression_x = 0.0
    expression_y = 0.0
    expression_z = 0.0
  []
[]

[FunctorMaterials]
  [Mouse]
    type = MFEMGenericFunctorMaterial
    prop_names = 'massCoef lossCoef'
    prop_values = 'mass_coef_mouse loss_coef_mouse'
    block = 1
  []
  [Air]
    type = MFEMGenericFunctorMaterial
    prop_names = 'massCoef lossCoef'
    prop_values = 'mass_coef_air loss_coef_air'
    block = 2
  []
[]

[BCs]
  [tangential_E]
    type = MFEMComplexVectorTangentialDirichletBC
    variable = E
    boundary = '2 3 4'
    vector_coefficient_real = vecZero
    vector_coefficient_imag = vecZero
  []
  [WaveguidePortIn]
    type = MFEMRWTE10IntegratedBC
    variable = E
    boundary = '5'
    input_port = true
    port_length_vector = "24.76e-2 0.0 0.0"
    port_width_vector = "0.0 12.38e-2 0.0"
    frequency = 9e8
    electric_permittivity = 8.8541878176e-12
    magnetic_permeability = 1.25663706127e-6
  []
  [WaveguidePortOut]
    type = MFEMRWTE10IntegratedBC
    variable = E
    boundary = '6'
    input_port = false
    port_length_vector = "24.76e-2 0.0 0.0"
    port_width_vector = "0.0 12.38e-2 0.0"
    frequency = 9e8
    electric_permittivity = 8.8541878176e-12
    magnetic_permeability = 1.25663706127e-6
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMComplexKernel
    variable = E
    [real_part]
      type = MFEMCurlCurlKernel
      coefficient = magnetic_reluctivity
    []
    [imag_part]
      type = MFEMCurlCurlKernel
      coefficient = 0.0
    []
  []
  [mass_loss]
    type = MFEMComplexKernel
    variable = E
    [real_part]
      type = MFEMVectorFEMassKernel
      coefficient = massCoef
    []
    [imag_part]
      type = MFEMVectorFEMassKernel
      coefficient = lossCoef
    []
  []
[]

[Solver]
  type = MFEMSuperLU
[]

[Executioner]
  type = MFEMSteady
  numeric_type = complex
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/ComplexWaveguide
    vtk_format = ASCII
  []
[]
