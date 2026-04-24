# Solve for the magnetic field around a closed superconductor subject to
# global current constraint.

conductor_domains = 'TorusCore TorusSheath'
vacuum_permeability = 1.0
n_value = 20.0
j_c = 1.0
e_c = 1.0

[Problem]
  type = MFEMProblem
[]

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/split_embedded_concentric_torus.e
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
    submesh = conductor
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
    source = coil_induced_h_field
    scale_factor = 1.0
    execute_on = TIMESTEP_END
  []
[]

[Functions]
  [resistivity]
    type = MFEMParsedFunction
    expression = '(e_c/j_c) * (j/j_c)^(n_val-1)'
    symbol_names = 'j e_c j_c n_val'
    symbol_values = 'coil_induced_h_field_curl_mag ${e_c} ${j_c} ${n_value}'
  []
  [j_dresistivity_dj]
    type = MFEMParsedFunction
    expression = '(n_val-1) * resistivity'
    symbol_names = 'n_val resistivity'
    symbol_values = '${n_value} resistivity'
  []
[]

[FunctorMaterials]
  [Vacuum]
    type = MFEMGenericFunctorMaterial
    prop_names = 'permeability'
    prop_values = '${vacuum_permeability}'
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
    type = MFEMNLCurlCurlKernel
    variable = coil_induced_h_field
    k_coefficient = resistivity
    curlu_dk_dcurlu_coefficient = j_dresistivity_dj
    block = ${conductor_domains}
  []
[]

[Solvers]
  [ams]
    type = MFEMHypreAMS
    fespace = CoilHCurlFESpace
  []
  [pcg]
    type = MFEMHyprePCG
    preconditioner = ams
    l_tol = 1e-12
    l_max_its = 100
    print_level = 0
  []
  [newton]
    type = MFEMNewtonNonlinearSolver
    max_its = 150
    abs_tol = 1e-5
    print_level = 1
  []
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
    type = MultiAppMFEMShapeEvaluationTransfer
    source_variables = h_field
    variables = coil_external_h_field
    from_multi_app = hphi_magnetostatic
  []
[]

[Postprocessors]
  [CoilPower]
    type = MFEMVectorFEInnerProductIntegralPostprocessor
    coefficient = resistivity
    dual_variable = j_field
    primal_variable = j_field
    block = 'TorusCore TorusSheath'
  []
[]

[Outputs]
  inactive = ConductorParaViewDataCollection
  [ReportedPostprocessors]
    type = CSV
    file_base = OutputData/HPhiMagnetodynamicNLClosedCoilCSV
  []
  [ConductorParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/HPhiMagnetodynamicNLClosedCoil
    vtk_format = ASCII
    submesh = conductor
  []
[]
