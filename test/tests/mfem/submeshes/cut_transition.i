# Subapp to constrain the source electric field on a thick cut of
# a closed toroidal conductor

[Mesh]
  type = MFEMMesh
  file = ../mesh/embedded_concentric_torus.e
[]

[Problem]
  type = MFEMProblem
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
  [SubMeshH1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
    submesh = cut
  []
  [SubMeshHCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
    submesh = cut
  []
[]

[Variables]
  [submesh_potential]
    type = MFEMVariable
    fespace = SubMeshH1FESpace
  []
[]

[AuxVariables]
  [potential]
    type = MFEMVariable
    fespace = H1FESpace
  []
  [external_e_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
  [submesh_external_e_field]
    type = MFEMVariable
    fespace = SubMeshHCurlFESpace
  []
[]

[AuxKernels]
  [grad]
    type = MFEMGradAux
    variable = submesh_external_e_field
    source = submesh_potential
    scale_factor = -1.0
    execute_on = TIMESTEP_END
  []
[]

[BCs]
  [transition_domain_boundary]
    type = MFEMScalarDirichletBC
    variable = submesh_potential
    boundary = transition_bdr
    coefficient = 0.0
  []
  [cut_surface]
    type = MFEMScalarDirichletBC
    variable = submesh_potential
    boundary = 'Cut'
    coefficient = -1.0 # Imposed global constraint on loop voltage
  []
[]

[FunctorMaterials]
  [Substance]
    type = MFEMGenericFunctorMaterial
    prop_names = diffusivity
    prop_values = 1.0
  []
[]

[Kernels]
  [submesh_diff]
    type = MFEMDiffusionKernel
    variable = submesh_potential
    coefficient = diffusivity
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Solver]
  type = MFEMHyprePCG
  preconditioner = boomeramg
  l_tol = 1e-8
  l_max_its = 1000
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Transfers]
  [submesh_transfer]
    type = MFEMSubMeshTransfer
    from_variable = submesh_external_e_field
    to_variable = external_e_field
    execution_order_group=2
  []
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/CutPotential
    vtk_format = ASCII
    submesh = cut
  []
  [ParaViewDataCollection2]
    type = MFEMParaViewDataCollection
    file_base = OutputData/WholePotential
    vtk_format = ASCII
  []
[]
