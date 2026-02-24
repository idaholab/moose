[Mesh]
  type = MFEMMesh
  file = ../mesh/square.msh
  nonconforming = true
[]

[Adaptivity]
  [Indicators]
    [l2zz]
      type     = MFEMZienkiewiczZhuIndicator
      variable = concentration
      kernel   = diff
      outputs  = none
    []
  []
  [Markers]
    [ref]
      type = MFEMRefinementMarker
      refine = 0.7 # error
      indicator = l2zz
      max_h_level = 1
      max_p_level = 1
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
    coefficient = 1.0
  []
  [low_terminal]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = '2'
    coefficient = 0.0
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = concentration
    coefficient = 1.0
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
  l_tol = 1e-16
  l_abs_tol = 1e-16
  l_max_its = 1000
[]

[Executioner]
  type       = MFEMSteady
  device     = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/Diffusion_amr
    vtk_format = ASCII
  []
[]
