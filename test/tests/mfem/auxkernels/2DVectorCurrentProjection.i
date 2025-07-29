[Mesh]
  type = MFEMMesh
  file = ../mesh/2Dwire.e
  dim = 2
  # uniform_refine = 1
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []

  [H1VectorFESpace]
    type = MFEMVectorFESpace
    fec_type = H1
    fec_order = FIRST
  []

  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []
[]

[Variables]
    [current]
        type = MFEMVariable
        fespace = H1FESpace
    []
    [Az]
        type = MFEMVariable
        fespace = H1FESpace
  
    []


[]

[AuxVariables]
    [grad_Az]
        type = MFEMVariable
        fespace = HCurlFESpace
    []

    [J]
        type = MFEMVariable
        fespace = H1VectorFESpace
    []
[]

[AuxKernels]
  [grad]
    type = MFEMGradAux
    variable = grad_Az
    source = Az
    execute_on = TIMESTEP_END
  []

   [current_output]
    type = MFEMVectorProjectAux
    coefficient = Jvalue 
    variable = J
  []


[]

[BCs]
    [essential]
        type = MFEMScalarDirichletBC
        variable = Az
        boundary = 1
        coefficient = 1
    []
[]

[FunctorMaterials]
    [with_current]
        type = MFEMGenericFunctorVectorMaterial
        prop_names = 'Jvalue'
        prop_values = '{8.0 8.0 8.0}'
        block = 2
    []

    [no_current]
        type = MFEMGenericFunctorVectorMaterial
        prop_names = 'Jvalue'
        prop_values = '{0.0 0.0 0.0}'
        block = 1
    []

  [Substance]
    type = MFEMGenericFunctorMaterial
    prop_names = diffusivity
    prop_values = 1.0
  []
[]

[Kernels]
    [diffusion]
        type = MFEMDiffusionKernel
        variable = Az 
        coefficient = diffusivity
    []

    [source]
        type = MFEMVectorDomainLFKernel
        variable = Az
        vector_coefficient = 'Jvalue'
    []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Solver]
  type = MFEMHyprePCG
  preconditioner= boomeramg
  l_tol = 1e-16
  l_max_its = 1000
  print_level = 2
[]

[Executioner]
  type = MFEMSteady
  device = "cpu"
[]


[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/2DVectorCurrentProjection
    vtk_format = ASCII
  []
  [VisItDataCollection]
    type = MFEMVisItDataCollection
    file_base = OutputData/VisItDataCollection
  []
[]