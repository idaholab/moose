# Magnetostatic problem solved on a closed conductor subject to
# global loop voltage constraint.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/embedded_concentric_torus.e
[]

[Problem]
  type = MFEMProblem
[]

[SubMeshes]
  inactive = 'fluxcut'
  [fluxcut]
    type = MFEMCutTransitionSubMesh
    cut_boundary = 'MeasurementPlane'
    block = 'TorusCore TorusSheath'
    transition_subdomain = transition_dom
    transition_subdomain_boundary = transition_bdr
    closed_subdomain = coil_dom
  []
[]

[FESpaces]
  inactive = 'FluxFESpace'
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
  [FluxFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
    submesh = fluxcut
  []
[]

[Variables]
  [a_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[AuxVariables]
  inactive = 'flux_e_field'
  [b_field]
    type = MFEMVariable
    fespace = HDivFESpace
  []
  [e_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
  [flux_e_field]
    type = MFEMVariable
    fespace = FluxFESpace
  []
[]

[AuxKernels]
  [curl]
    type = MFEMCurlAux
    variable = b_field
    source = a_field
    scale_factor = 1.0
    execute_on = TIMESTEP_END
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
  inactive = 'ConductorBoundary'
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
  [ConductorBoundary]
    type = MFEMGenericFunctorMaterial
    prop_names = conductivity_boundary
    prop_values = 1.0
    boundary = 'MeasurementPlane'
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


[Solvers]
  [ams]
    type = MFEMHypreAMS
    fespace = HCurlFESpace
  []
  [main]
    type = MFEMHyprePCG
    preconditioner = ams
    l_tol = 1e-14
    l_max_its = 1000
  []
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
  inactive = 'submesh_transfer_to_fluxsurface'
  [from_coil]
    type = MultiAppMFEMCopyTransfer
    source_variables = e_field
    variables = e_field
    from_multi_app = coil
  []
  [submesh_transfer_to_fluxsurface]
    type = MFEMSubMeshTransfer
    from_variable = e_field
    to_variable = flux_e_field
    execute_on = TIMESTEP_END
  []
[]

[Postprocessors]
  inactive = 'CoilCurrent'
  [CoilPower]
    type = MFEMVectorFEInnerProductIntegralPostprocessor
    coefficient = conductivity
    dual_variable = e_field
    primal_variable = e_field
    block = 'TorusCore TorusSheath'
  []
  [CoilCurrent]
    type = MFEMVectorBoundaryFluxIntegralPostprocessor
    coefficient = conductivity_boundary
    variable = flux_e_field
    boundary = 'MeasurementPlane'
  []
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/MagnetostaticClosedCoil
    vtk_format = ASCII
  []
  [ReportedPostprocessors]
    type = CSV
    file_base = OutputData/AVMagnetostaticClosedCoilCSV
  []
[]
