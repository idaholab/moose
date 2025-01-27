# 2D irrotational vortex with Nedelec elements of the first kind.

centre_x = 0.25
centre_y = 0.5

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
    type = MFEMFESpace
    fec_type = H1
    fec_order = FIRST
    dim = 2
  []
  [HCurlFESpace]
    type = MFEMFESpace
    fec_type = ND
    fec_order = FIRST
    dim = 2
  []
[]

[Variables]
  [test_var]
    type = MFEMVariable
    fespace = H1FESpace
  []
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
  [tangential_velocity_boundary]
    type = MFEMVectorFunctionTangentialDirichletBC
    variable = velocity
    boundary = '1'
    function = exact_velocity
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
  [curl]
    type = MFEMMixedScalarCurlKernel
    variable = test_var
    coefficient = one
    trial_variable = velocity
  []
[]

[Preconditioner]
  [ams]
    type = MFEMHypreAMS
    fespace = HCurlFESpace
  []
[]

[Solver]
  type = MFEMHypreGMRES
  preconditioner = ams
  l_tol = 1e-6  
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Postprocessors]
  [l2_error]
    type = MFEMVectorL2Error
    variable = velocity
    function = exact_velocity
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
