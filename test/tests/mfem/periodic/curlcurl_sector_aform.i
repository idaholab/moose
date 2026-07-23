# Magnetostatic problem solved on a sector of a closed conductor subject to
# global loop voltage constraint.

exterior_boundaries = "inside outside top bottom coil_primary_plane vacuum_primary_plane coil_replica_plane vacuum_replica_plane"
initial_coil_domains = 'coil'

[Problem]
  type = MFEMProblem
[]

[Mesh]
  type = MFEMMesh
  file = ../mesh/meshed_wedge_test.e
  rotational_symmetry_order = 12
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
    boundary = ${exterior_boundaries}
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
    block = ${initial_coil_domains}
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
    block = ${initial_coil_domains}
  []
[]

[Preconditioner]
  [ams]
    type = MFEMHypreAMS
    fespace = HCurlFESpace
  []
[]

[Solvers]
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
    input_files = curlcurl_sector_source.i
    execute_on = INITIAL
  []
[]

[Transfers]
  [from_coil]
    type = MultiAppMFEMCopyTransfer
    source_variables = e_field
    variables = e_field
    from_multi_app = coil
  []
[]

[Postprocessors]
  [solution_l2_norm]
    type = MFEMVectorL2Error
    variable = b_field
    function = 0
  []
[]

[Outputs]
  csv = true
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
