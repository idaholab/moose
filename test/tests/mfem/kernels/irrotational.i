# 2D irrotational vortex with Nedelec elements of the first kind.

centre_x = -0.75
centre_y = 0.1

[Mesh]
  type = MFEMMesh
  file = gold/vortex.msh
  dim = 2
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = SEVENTH
  []
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = SEVENTH
  []
[]

[Variables]
  [velocity_potential]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[AuxVariables]
  [velocity]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[Functions]
  [speed]
    type = ParsedFunction
    expression = '1 / sqrt((x-x0)^2 + (y-y0)^2)'
    symbol_names = 'x0 y0'
  symbol_values = '${centre_x} ${centre_y}'
  []
  [theta]
    type = ParsedFunction
    expression = 'atan2(y-y0, x-x0)'
    symbol_names = 'x0 y0'
    symbol_values = '${centre_x} ${centre_y}'
  []
  [exact_velocity]
    type = ParsedVectorFunction
    expression_x = '-v * sin(th)'
    expression_y = 'v * cos(th)'
    symbol_names = 'v th'
    symbol_values = 'speed theta'
  []
[]

[BCs]
  [potential_velocity_boundary]
    type = MFEMScalarFunctionDirichletBC
    variable = velocity_potential
    boundary = '1'
    function = theta
  []
[]

[Materials]
  [Substance]
    type = MFEMGenericConstantMaterial
    prop_names = one
    prop_values = 1.0
  []
[]

[Kernels]
  [laplacian]
    type = MFEMDiffusionKernel
    variable = velocity_potential
    coefficient = one
  []
[]

[AuxKernels]
  [grad]
    type = MFEMGradAux
    variable = velocity
    source = velocity_potential
    execute_on = TIMESTEP_END
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Solver]
  type = MFEMHypreGMRES
  preconditioner = boomeramg
  l_tol = 1e-16
  l_max_its = 1000
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Postprocessors]
  [potential_error]
    type = MFEML2Error
    variable = velocity_potential
    function = theta
    execution_order_group = 1
  []
  [velocity_error]
    type = MFEMVectorL2Error
    variable = velocity
    function = exact_velocity
    execution_order_group = 1
  []
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/Irrotational
    vtk_format = ASCII
  []
  [L2CSV]
    type = CSV
    file_base = OutputData/Irrotational
  []
[]
