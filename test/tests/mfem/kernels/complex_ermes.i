freq = 900e6
angfreq = 5654866776.46
epsilon0 = 8.8541878176e-12
mu0 = 1.256637061435917e-06
magnetic_reluctivity = 795774.715459
elec_cond_mouse = 0.97
elec_cond_air = 0.0

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
  [dielec_perm_mouse]
    type = ParsedFunction
    expression = 43*${epsilon0}
  []
  [mass_coef_mouse]
    type = ParsedFunction
    expression = -dielec_perm_mouse*${angfreq}^2
    symbol_names = 'dielec_perm_mouse'
    symbol_values = 'dielec_perm_mouse'
  []
  [loss_coef_mouse]
    type = ParsedFunction
    expression = ${angfreq}*${elec_cond_mouse}
  []
  [dielec_perm_air]
    type = ParsedFunction
    expression = ${epsilon0}
  []
  [mass_coef_air]
    type = ParsedFunction
    expression = -dielec_perm_air*${angfreq}^2
    symbol_names = dielec_perm_air
    symbol_values = dielec_perm_air
  []
  [loss_coef_air]
    type = ParsedFunction
    expression = ${angfreq}*${elec_cond_air}
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
    prop_names = 'massCoef lossCoef MagReluctivity'
    prop_values = 'mass_coef_mouse loss_coef_mouse ${magnetic_reluctivity}'
    block = 1
  []
  [Air]
    type = MFEMGenericFunctorMaterial
    prop_names = 'massCoef lossCoef MagReluctivity'
    prop_values = 'mass_coef_air loss_coef_air ${magnetic_reluctivity}'
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
    frequency = ${freq}
    electric_permittivity = ${epsilon0}
    magnetic_permeability = ${mu0}
  []
  [WaveguidePortOut]
    type = MFEMRWTE10IntegratedBC
    variable = E
    boundary = '6'
    input_port = false
    port_length_vector = "24.76e-2 0.0 0.0"
    port_width_vector = "0.0 12.38e-2 0.0"
    frequency = ${freq}
    electric_permittivity = ${epsilon0}
    magnetic_permeability = ${mu0}
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMComplexKernel
    variable = E
    [real_part]
      type = MFEMCurlCurlKernel
      coefficient = MagReluctivity
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
  assembly_level = legacy
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/ComplexERMES
    vtk_format = ASCII
  []
[]
