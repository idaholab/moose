[Mesh]
  type = MFEMMesh
  file = ../mesh/embedded_torus.e
[]

[Problem]
  type = MFEMProblem
[]

[SubMeshes]
  [cut]
    type = MFEMCutTransitionSubMesh
    cut_boundary = 1
    transition_subdomain = cut
    transition_subdomain_boundary = transition_bdr
    closed_subdomain = coil
    block = '1 2'
  []   
[]

[FESpaces]
  [SubMeshH1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
    submesh = cut
  []
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
  [SubMeshHCurlFESpace]
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
  [submesh_grad_source_potential]
    type = MFEMVariable
    fespace = SubMeshHCurlFESpace
  []    
  [grad_source_potential]
    type = MFEMVariable
    fespace = HCurlFESpace
  []  
[]

[AuxKernels]
  [grad]
    type = MFEMGradAux
    variable = submesh_grad_source_potential
    source = submesh_potential
    scale_factor = 1.0
    execute_on = TIMESTEP_END
  []
[]

# High terminal set to new bdr attribute
[BCs]
  [high_terminal]
    type = MFEMScalarDirichletBC
    variable = submesh_potential
    boundary = transition_bdr
    coefficient = 500.0
  []
  [low_terminal]
    type = MFEMScalarDirichletBC
    variable = submesh_potential
    boundary = '1'
    coefficient = -500.0
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
    from_variable = submesh_grad_source_potential
    to_variable = grad_source_potential
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
