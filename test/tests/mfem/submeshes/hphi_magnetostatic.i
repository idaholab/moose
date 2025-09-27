# Solve for the magnetic field around a closed conductor subject to
# global current constraint.

initial_vacuum_domains = 'Exterior'
vacuum_cut_surface = 'Cut'
conductor_current = 1.0
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
    prop_names = permeability
    prop_values = ${vacuum_permeability}
  []
[]

[ICs]
  [vacuum_cut_potential_ic]
    type = MFEMScalarBoundaryIC
    variable = vacuum_cut_potential
    boundary = ${vacuum_cut_surface}
    coefficient = ${conductor_current}
  []
[]

[SubMeshes]
  [cut]
    type = MFEMCutTransitionSubMesh
    cut_boundary = ${vacuum_cut_surface}
    block = ${initial_vacuum_domains}
    transition_subdomain = transition_dom
    transition_subdomain_boundary = transition_bdr
    closed_subdomain = vacuum_dom
  []
  [vacuum]
    type = MFEMDomainSubMesh
    block = vacuum_dom
    execution_order_group = 2
  []
[]

[FESpaces]
  [VacuumH1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
    submesh = vacuum
  []
  [VacuumHCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
    submesh = vacuum
  []
  [TransitionH1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
    submesh = cut
  []
  [TransitionHCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
    submesh = cut
  []
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []
[]

[Variables]
  [vacuum_magnetic_potential]
    type = MFEMVariable
    fespace = VacuumH1FESpace
  []
[]

[AuxVariables]
  [vacuum_cut_potential]
    type = MFEMVariable
    fespace = VacuumH1FESpace
  []
  [transition_cut_potential]
    type = MFEMVariable
    fespace = TransitionH1FESpace
  []
  [transition_cut_function_field]
    type = MFEMVariable
    fespace = TransitionHCurlFESpace
  []
  [background_h_field]
    type = MFEMVariable
    fespace = VacuumHCurlFESpace
  []
  [cut_function_field]
    type = MFEMVariable
    fespace = VacuumHCurlFESpace
  []
  [vacuum_h_field]
    type = MFEMVariable
    fespace = VacuumHCurlFESpace
  []
  [h_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[AuxKernels]
  [update_background_h_field]
    type = MFEMGradAux
    variable = background_h_field
    source = vacuum_magnetic_potential
    scale_factor = -1.0
    execute_on = TIMESTEP_END
  []
  [update_transition_cut_function_field]
    type = MFEMGradAux
    variable = transition_cut_function_field
    source = transition_cut_potential
    scale_factor = -1.0
    execute_on = TIMESTEP_END
  []
  [update_total_h_field]
    type = MFEMSumAux
    variable = vacuum_h_field
    source_variables = 'background_h_field cut_function_field'
    execute_on = TIMESTEP_END
    execution_order_group = 3
  []
[]

[BCs]
  # Set zero of magnetic potential on symmetry plane
  [Exterior]
    type = MFEMScalarDirichletBC
    variable = vacuum_magnetic_potential
    boundary = 'Cut'
    coefficient = 0.0
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = vacuum_magnetic_potential
    coefficient = permeability
  []
  [source]
    type = MFEMMixedGradGradKernel
    trial_variable = vacuum_cut_potential
    variable = vacuum_magnetic_potential
    coefficient = permeability
    block = 'transition_dom'
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
  l_tol = 1e-8
  l_max_its = 100
[]

[Executioner]
  type = MFEMSteady
[]

[Transfers]
  [submesh_transfer_to_transition]
    type = MFEMSubMeshTransfer
    from_variable = vacuum_cut_potential
    to_variable = transition_cut_potential
    execute_on = TIMESTEP_END
  []
  [submesh_transfer_from_transition]
    type = MFEMSubMeshTransfer
    from_variable = transition_cut_function_field
    to_variable = cut_function_field
    execute_on = TIMESTEP_END
    execution_order_group = 2
  []
  [submesh_transfer_from_vacuum]
    type = MFEMSubMeshTransfer
    from_variable = vacuum_h_field
    to_variable = h_field
    execute_on = TIMESTEP_END
    execution_order_group = 4
  []
[]

[Postprocessors]
  [MagneticEnergy]
    type = MFEMVectorFEInnerProductIntegralPostprocessor
    coefficient = ${fparse 0.5*vacuum_permeability}
    dual_variable = vacuum_h_field
    primal_variable = vacuum_h_field
    execution_order_group = 4
    block = 'Exterior'
  []
[]

[Outputs]
  [ReportedPostprocessors]
    type = CSV
    file_base = OutputData/HPhiMagnetostaticClosedCoilCSV
  []
  [VacuumParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/HPhiMagnetostaticClosedCoil
    vtk_format = ASCII
    submesh = vacuum
  []
[]
