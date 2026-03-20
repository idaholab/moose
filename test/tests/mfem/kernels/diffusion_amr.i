[Mesh]
  type = MFEMMesh
  file = ../mesh/square.msh
  nonconforming = true
[]

[Adaptivity]
  [Indicators]
    [l2zz]
      type = MFEML2ZienkiewiczZhuIndicator
      variable = concentration
      kernel = diff
    []
  []
  [Markers]
    [ref]
      type = MFEMRefinementMarker
      threshold = 0.7
      indicator = l2zz
      max_h_level = 1
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
  [top]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = 4
    coefficient = 1
  []
  [bottom]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = 2
  []
[]

[Functions]
  [D]
    type = ParsedFunction
    expression = 1+1/(1+exp(20*y-10))
  []
  [solution]
    type = ParsedFunction
    expression = (20*y+log(exp(20*y)+2*exp(10))-log(1+2*exp(10)))/(30+log(2+exp(10))-log(1+2*exp(10)))
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = concentration
    coefficient = D
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
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Postprocessors]
  [error]
    type = MFEML2Error
    variable = concentration
    function = solution
  []
[]

[Outputs]
  csv = true
  file_base = OutputData/DiffusionHRefinement
[]
