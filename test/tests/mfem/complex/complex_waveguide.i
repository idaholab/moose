freq = 900e6
angfreq = ${fparse 2*pi*freq}
epsilon0 = 8.8541878176e-12
mu0 = ${fparse 4e-7*pi}
magnetic_reluctivity = ${fparse 1/mu0}
elec_cond_mouse = 0.97
elec_cond_air = 1e-323

[Mesh]
  type = MFEMMesh
  file = ../mesh/waveguide.g
  dim = 3
[]

[Problem]
  type = MFEMProblem
  numeric_type = complex
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
  [mass_coef_mouse]
    type = ParsedFunction
    expression = -43*${epsilon0}*${angfreq}^2
  []
  [loss_coef_mouse]
    type = ParsedFunction
    expression = ${angfreq}*${elec_cond_mouse}
  []
  [mass_coef_air]
    type = ParsedFunction
    expression = -${epsilon0}*${angfreq}^2
  []
  [loss_coef_air]
    type = ParsedFunction
    expression = ${angfreq}*${elec_cond_air}
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
  []
  [WaveguidePortIn]
    type = MFEMRWTE10IntegratedBC
    variable = E
    boundary = '5'
    input_port = true
    port_length_vector = "24.76e-2 0.0 0.0"
    port_width_vector = "0.0 12.38e-2 0.0"
    frequency = ${freq}
    epsilon = ${epsilon0}
    mu = ${mu0}
  []
  [WaveguidePortOut]
    type = MFEMRWTE10IntegratedBC
    variable = E
    boundary = '6'
    input_port = false
    port_length_vector = "24.76e-2 0.0 0.0"
    port_width_vector = "0.0 12.38e-2 0.0"
    frequency = ${freq}
    epsilon = ${epsilon0}
    mu = ${mu0}
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMComplexKernel
    variable = E
    [RealComponent]
      type = MFEMCurlCurlKernel
      coefficient = MagReluctivity
    []
  []
  [mass_loss]
    type = MFEMComplexKernel
    variable = E
    [RealComponent]
      type = MFEMVectorFEMassKernel
      coefficient = massCoef
    []
    [ImagComponent]
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
  assembly_level = legacy
[]

[Postprocessors]
  [ObstructionAbsorption]
    type = MFEMComplexVectorPeriodAveragedPostprocessor
    coefficient = ${elec_cond_mouse}
    dual_variable = E
    primal_variable = E
    block = 1
  []
[]


[Outputs]
  [ReportedPostprocessors]
    type = CSV
    file_base = OutputData/ComplexWaveguide
  []
[]
