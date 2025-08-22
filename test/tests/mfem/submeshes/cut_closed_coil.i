# Solve for the electric field on a closed conductor subject to
# global loop voltage constraint.

[Problem]
  type = MFEMProblem
[]

[Mesh]
    type = MFEMMesh
    file = ../mesh/embedded_concentric_torus.e
[]

[SubMeshes]
  [cut]
    type = MFEMCutTransitionSubMesh
    cut_boundary = 'Cut'
    block = 'TorusCore TorusSheath'
    transition_subdomain = transition_dom
    transition_subdomain_boundary = transition_bdr
    closed_subdomain = coil
  []
  [coil]
    type = MFEMDomainSubMesh
    block = coil
    execution_order_group = 2
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
  [CoilH1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
    submesh = coil
  []
  [CoilHCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
    submesh = coil
  []
[]

[Variables]
  [coil_induced_potential]
    type = MFEMVariable
    fespace = CoilH1FESpace
  []
[]

[AuxVariables]
  [induced_potential]
    type = MFEMVariable
    fespace = H1FESpace
  []
  [induced_e_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
  [coil_external_e_field]
    type = MFEMVariable
    fespace = CoilHCurlFESpace
  []
  [external_e_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
  [e_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[AuxKernels]
  [grad]
    type = MFEMGradAux
    variable = induced_e_field
    source = induced_potential
    scale_factor = -1.0
    execute_on = TIMESTEP_END
    execution_order_group = 1
  []
  [sum]
    type = MFEMSumAux
    variable = e_field
    source1 = induced_e_field
    source2 = external_e_field
    scale_factor = 1.0
    execute_on = TIMESTEP_END
    execution_order_group = 2
  []
[]

[FunctorMaterials]
  [Substance]
    type = MFEMGenericFunctorMaterial
    prop_names = conductivity
    prop_values = 1.0
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = coil_induced_potential
    coefficient = conductivity
  []
  [source]
    type = MFEMMixedVectorGradientKernel
    trial_variable = coil_external_e_field
    variable = coil_induced_potential
    coefficient = -1.0
    transpose = true
    block = 'transition_dom'
  []
[]

[Solver]
  type = MFEMSuperLU
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[MultiApps]
  [transition]
    type = FullSolveMultiApp
    input_files = cut_transition.i
    execute_on = INITIAL
  []
[]

[Transfers]
  [from_sub]
    type = MultiAppMFEMCopyTransfer
    source_variable = external_e_field
    variable = external_e_field
    from_multi_app = transition
  []
  [submesh_transfer_to_coil]
    type = MFEMSubMeshTransfer
    from_variable = external_e_field
    to_variable = coil_external_e_field
    execute_on = INITIAL
  []
  [submesh_transfer_from_coil]
    type = MFEMSubMeshTransfer
    from_variable = coil_induced_potential
    to_variable = induced_potential
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/CutPotentialCoil
    vtk_format = ASCII
    submesh = cut
  []
  [ParaViewDataCollection2]
    type = MFEMParaViewDataCollection
    file_base = OutputData/WholePotentialCoil
    vtk_format = ASCII
  []
  [ParaViewDataCollection3]
    type = MFEMParaViewDataCollection
    file_base = OutputData/Coil
    vtk_format = ASCII
    submesh = coil
  []
[]
