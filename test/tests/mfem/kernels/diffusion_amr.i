[Mesh]
  type = MFEMMesh
  file = ../mesh/mug.e
  dim = 3
[]

[Adaptivity]
  marker = errormarker
  steps  = 2

  [Indicators]
    [l2zz]
      type     = MFEMZienkiewiczZhuEstimator
      variable = concentration
      kernel   = diff
      fe_space = H1FESpace
      outputs  = none
    []
  []

  [Markers]
    [errormarker]
      type = ErrorFractionMarker
      indicator = l2zz
      refine = 0.5 # split/refine elements in the upper half of the indicator error range
      coarsen = 0 # dont do any coarsening
      outputs = none
    []
  []
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
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []  
[]

[Variables]
  [concentration]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[AuxVariables]
  [concentration_gradient]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[AuxKernels]
  [grad]
    type = MFEMGradAux
    variable = concentration_gradient
    source = concentration
    execute_on = TIMESTEP_END
  []
[]

[BCs]
  [bottom]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = '1'
    value = 1.0
  []
  [low_terminal]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = '2'
    value = 0.0
  []
[]

[FunctorMaterials]
  [Substance]
    type = MFEMGenericConstantFunctorMaterial
    prop_names = diffusivity
    prop_values = 1.0
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = concentration
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
  l_tol = 1e-16
  l_abs_tol = 1e-16
  l_max_its = 1000  
[]

[Executioner]
  type       = MFEMAMR
  device     = cpu
  variable   = concentration
  kernel     = diff
  fe_space   = H1FESpace
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/Diffusion_amr
    vtk_format = ASCII
  []
[]
