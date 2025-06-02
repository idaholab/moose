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
  [exact_pressure_rhs]
    type = ParsedFunction
    expression = '-exp(x) * sin(y)'
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
    type = MFEMVectorFEBoundaryFluxIntegratedBC
    variable = velocity
    coefficient = exact_pressure_rhs
  []
[]

[Kernels]
  [VelocityMass]
    type = MFEMVectorFEMassKernel
    variable = velocity
  []
  [PressureGrad]
    type = MFEMVectorFEDivergenceKernel
    trial_variable = pressure
    variable = velocity
    coefficient = -1
    transpose = true
  []
  [VelocityDiv]
    type = MFEMVectorFEDivergenceKernel
    trial_variable = velocity
    variable = pressure
    coefficient = -1
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
  []
  [pressure_error]
    type = MFEML2Error
    variable = pressure
    function = exact_pressure
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
