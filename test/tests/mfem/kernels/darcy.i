[Mesh]
  type = MFEMMesh
  file = ../mesh/star.mesh
  uniform_refine = 4
[]

[Problem]
  type = MFEMProblem
[]

[Functions]
  [exact_velocity]
    type = ParsedVectorFunction
    expression_x = '-exp(x) * sin(y)'
    expression_y = '-exp(x) * cos(y)'
  []
  [exact_pressure]
    type = ParsedFunction
    expression = 'exp(x) * sin(y)'
  []
  [f_bdr]
    type = ParsedFunction
    expression = '-exp(x) * sin(y)'
  []
  [g_force]
    type = ParsedFunction
    expression = '0.0'
  []
[]

[FESpaces]
  [HDivFESpace]
    type = MFEMVectorFESpace
    fec_type = RT
    fec_order = SECOND
  []
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = SECOND
  []
[]

[Variables]
  [velocity]
    type = MFEMVariable
    fespace = HDivFESpace
  []
  [pressure]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[BCs]
  [flux_boundaries]
    type = MFEMScalarFunctorFEFluxIntegratedBC
    variable = velocity
    coefficient = f_bdr
    boundary = 1
  []
[]

[Kernels]
  [VelocityMass]
    type = MFEMVectorFEMassKernel
    variable = velocity
    coefficient = one
  []
  [PressureGrad]
    type = MFEMVectorFEDivergenceKernel
    trial_variable = pressure
    variable = velocity
    coefficient = mone
    transpose = true
  []
  [VelocityDiv]
    type = MFEMVectorFEDivergenceKernel
    trial_variable = velocity
    variable = pressure
    coefficient = mone
  []
  [PressureForcing]
    type = MFEMDomainLFKernel
    coefficient = g_force
    variable = pressure
  []
[]

[FunctorMaterials]
  [Substance]
    type = MFEMGenericConstantFunctorMaterial
    prop_names = 'one mone'
    prop_values = '1.0 -1.0'
  []
[]

[Solver]
  type = MFEMSuperLU
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Postprocessors]
  [velocity_error]
    type = MFEMVectorL2Error
    variable = velocity
    function = exact_velocity
    execution_order_group = 1
  []
  [pressure_error]
    type = MFEML2Error
    variable = pressure
    function = exact_pressure
    execution_order_group = 1
  []
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/Darcy
    vtk_format = ASCII
  []
  [DarcyErrorCSV]
    type = CSV
    file_base = OutputData/Darcy
  []
[]
