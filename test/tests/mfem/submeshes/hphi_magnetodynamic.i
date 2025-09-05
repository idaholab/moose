# Solve for the magnetic field around a closed conductor subject to
# global current constraint.

conductor_domains = 'TorusCore TorusSheath'
conductor_resistivity = 1.0
vacuum_permeability = 1.0

[Problem]
  type = MFEMProblem
[]

[Mesh]
    type = MFEMMesh
    file = ../mesh/split_embedded_concentric_torus.e
[]

[FunctorMaterials]
  [Conductor]
    type = MFEMGenericFunctorMaterial
    prop_names = 'resistivity'
    prop_values = ${conductor_resistivity}
    block = ${conductor_domains}
  []
  [Vacuum]
    type = MFEMGenericFunctorMaterial
    prop_names = 'permeability'
    prop_values = '${vacuum_permeability}'
  []
[]

[SubMeshes]
  [conductor]
    type = MFEMDomainSubMesh
    block = ${conductor_domains}
    submesh_boundary = conductor_surface
  []
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
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
  [CoilHCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
    submesh = conductor
  []
[]

[Variables]
  [coil_induced_h_field]
    type = MFEMVariable
    fespace = CoilHCurlFESpace
  []
[]

[AuxVariables]
  [h_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
  [coil_external_h_field]
    type = MFEMVariable
    fespace = CoilHCurlFESpace
  []
  [j_field]
    type = MFEMVariable
    fespace = HDivFESpace
  []
[]

[AuxKernels]
  [update_j_field]
    type = MFEMCurlAux
    variable = j_field
    source = h_field
    scale_factor = 1.0
    execute_on = TIMESTEP_END
    execution_order_group = 4
  []
[]

[BCs]
  [conductor_bdr]
    type = MFEMVectorTangentialDirichletBC
    variable = coil_induced_h_field
    vector_coefficient = coil_external_h_field
    boundary = conductor_surface
  []
[]

[Kernels]
  [dBdt]
    type = MFEMTimeDerivativeVectorFEMassKernel
    variable = coil_induced_h_field
    coefficient = permeability
  []
  [curlE]
    type = MFEMCurlCurlKernel
    variable = coil_induced_h_field
    coefficient = resistivity
  []
[]

[Preconditioner]
  [ams]
    type = MFEMHypreAMS
    fespace = CoilHCurlFESpace
  []
[]

[Solver]
  type = MFEMHyprePCG
  preconditioner = ams
  l_tol = 1e-9
  l_max_its = 100
[]

[Executioner]
  type = MFEMTransient
  dt = 0.5
  start_time = 0.0
  end_time = 2.0
[]

[MultiApps]
  [hphi_magnetostatic]
    type = FullSolveMultiApp
    input_files = hphi_magnetostatic.i
    execute_on = INITIAL
  []
[]

[Transfers]
  [from_external_field]
    type = MultiAppMFEMCopyTransfer
    source_variable = h_field
    variable = h_field
    from_multi_app = hphi_magnetostatic
  []
  [submesh_transfer_to_coil]
    type = MFEMSubMeshTransfer
    from_variable = h_field
    to_variable = coil_external_h_field
    execute_on = TIMESTEP_BEGIN
  []
  [submesh_transfer_from_coil]
    type = MFEMSubMeshTransfer
    from_variable = coil_induced_h_field
    to_variable = h_field
    execute_on = TIMESTEP_END
  []
[]

[Postprocessors]
  [CoilPower]
    type = MFEMVectorFEInnerProductIntegralPostprocessor
    coefficient = resistivity
    dual_variable = j_field
    primal_variable = j_field
    execution_order_group = 5
    block = 'TorusCore TorusSheath'
  []
[]

[Outputs]
  [ReportedPostprocessors]
    type = CSV
    file_base = OutputData/HPhiMagnetodynamicClosedCoilCSV
  []
  [VacuumParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/HPhiMagnetodynamicClosedCoil
    vtk_format = ASCII
  []
[]
