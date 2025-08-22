# Magnetostatic problem solved on a closed conductor subject to
# global loop voltage constraint.

[Mesh]
  type = MFEMMesh
  file = ../mesh/embedded_concentric_torus.e
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
  [a_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[AuxVariables]
  [b_field]
    type = MFEMVariable
    fespace = HDivFESpace
  []
  [e_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[AuxKernels]
  [curl]
    type = MFEMCurlAux
    variable = b_field
    source = a_field
    scale_factor = 1.0
    execute_on = TIMESTEP_END
    execution_order_group = 3
  []
[]

[Functions]
  [exact_a_field]
    type = ParsedVectorFunction
    expression_x = '0'
    expression_y = '0'
    expression_z = '0'
  []
[]

[BCs]
  [tangential_a_bdr]
    type = MFEMVectorTangentialDirichletBC
    variable = a_field
    vector_coefficient = exact_a_field
    boundary = 'Exterior'
  []
[]

[FunctorMaterials]
  [Vacuum]
    type = MFEMGenericFunctorMaterial
    prop_names = reluctivity
    prop_values = 1.0
  []
  [Conductor]
    type = MFEMGenericFunctorMaterial
    prop_names = conductivity
    prop_values = 1.0
    block = 'TorusCore TorusSheath'
  []
[]

[Kernels]
  [mass]
    type = MFEMVectorFEMassKernel
    variable = a_field
    coefficient = 1e-10
  []
  [curlcurl]
    type = MFEMCurlCurlKernel
    variable = a_field
    coefficient = reluctivity
  []
  [source]
    type = MFEMMixedVectorMassKernel
    variable = a_field
    trial_variable = e_field
    coefficient = conductivity
    block = 'TorusCore TorusSheath'
  []
[]

[Preconditioner]
  [ams]
    type = MFEMHypreAMS
    fespace = HCurlFESpace
  []
[]

[Solver]
  type = MFEMHyprePCG
  preconditioner = ams
  l_tol = 1e-14
  l_max_its = 1000
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[MultiApps]
  [coil]
    type = FullSolveMultiApp
    input_files = cut_closed_coil.i
    execute_on = INITIAL
  []
[]

[Transfers]
  [from_sub]
    type = MultiAppMFEMCopyTransfer
    source_variable = e_field
    variable = e_field
    from_multi_app = coil
  []
[]

[Postprocessors]
  [TotalCurrent]
    type = MFEMBoundaryNetFluxPostprocessor
    variable = e_field
    boundary = 'MeasurementPlane'
  []
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/MagnetostaticClosedCoil
    vtk_format = ASCII
  []
  [ReportedCurrent]
    type = CSV
    file_base = OutputData/MagnetostaticClosedCoilCSV
  []
[]
