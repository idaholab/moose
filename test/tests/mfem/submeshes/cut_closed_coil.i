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
    cut_name = cut
    boundary = 1
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
  [potential]
    type = MFEMVariable
    fespace = H1FESpace
  [] 
[]

[AuxVariables]
  [grad_potential]
    type = MFEMVariable
    fespace = HCurlFESpace
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
    variable = potential
    coefficient = diffusivity
  []
  [source]
    type = MFEMDomainLFGradKernel
    variable = potential
    vector_coefficient = grad_source_potential
    block = cut
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
[]
