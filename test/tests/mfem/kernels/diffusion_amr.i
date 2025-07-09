[Mesh]
  type = MFEMMesh
  file = ../mesh/mug.e
  dim = 3
[]

[Adaptivity]
  [Indicators]
    [l2zz]
      type     = MFEMZienkiewiczZhuEstimator
      variable = concentration
      kernel   = diff
      fe_space = H1FESpace # not necessary. Get the fespace from the variable
      outputs  = none
    []
  []
  [Markers]
    [ref]
      type = MFEMRefiner
#      indicator=l2zz
      refine = 0.7 # error 
      steps  = 2 # total number of refinement steps
      max_h_level = 2
      max_p_level = 2
#      switch_h_to_p_refinement = enum with h/p/h-p refinement # add this one later
#
#
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
  type = MFEMHyprePCG
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
