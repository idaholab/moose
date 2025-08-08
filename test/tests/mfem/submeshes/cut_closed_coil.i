[Problem]
  type = MFEMProblem
[]

[Mesh]
    type = MFEMMesh
    file = ../mesh/embedded_torus.e
[]

[SubMeshes]
  [cut]
    type = MFEMCutTransitionSubMesh
    cut_boundary = 1
    transition_subdomain = cut_test
    transition_subdomain_boundary = transition_bdr
    closed_subdomain = coil
    block = '1 2'
  []  
  [coil]
    type = MFEMDomainSubMesh
    block = coil
    execution_order_group = 2
  []  
[]

[FESpaces]
  [CoilH1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
    submesh = coil
  []  
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
  [CoilHCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
    submesh = coil
  []
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []
[]

[Variables]
  [coil_potential]
    type = MFEMVariable
    fespace = CoilH1FESpace
  [] 
[]

[AuxVariables]
  [potential]
    type = MFEMVariable
    fespace = H1FESpace
  []   
  [grad_potential]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
  [coil_grad_source_potential]
    type = MFEMVariable
    fespace = CoilHCurlFESpace
  []  
  [grad_source_potential]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
  [current_density]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[AuxKernels]
  [grad]
    type = MFEMGradAux
    variable = grad_potential
    source = potential
    scale_factor = 1.0
    execute_on = TIMESTEP_END
    execution_order_group = 1
  []
  [sum]
    type = MFEMSumAux
    variable = current_density
    source1 = grad_potential
    source2 = grad_source_potential
    scale_factor = -1.0
    execute_on = TIMESTEP_END
    execution_order_group = 2
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
  [diff]
    type = MFEMDiffusionKernel
    variable = coil_potential
    coefficient = diffusivity
  []
  [source]
    type = MFEMDomainLFGradKernel
    variable = coil_potential
    vector_coefficient = coil_grad_source_potential
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

[MultiApps]
  [subapp]
    type = FullSolveMultiApp
    input_files = cut_transition.i
    execute_on = INITIAL
  []
[]

[Transfers]
  [from_sub]
    type = MultiAppMFEMCopyTransfer
    source_variable = grad_source_potential
    variable = grad_source_potential
    from_multi_app = subapp
  []
  [submesh_transfer_to_coil]
    type = MFEMSubMeshTransfer
    from_variable = grad_source_potential
    to_variable = coil_grad_source_potential
    execute_on = INITIAL
  []
  [submesh_transfer_from_coil]
    type = MFEMSubMeshTransfer
    from_variable = coil_potential
    to_variable = potential
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
