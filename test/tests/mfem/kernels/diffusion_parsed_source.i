[Mesh]
  type = MFEMMesh
  file = ../mesh/mug.e
  dim = 3
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
[]

[Variables]
  [concentration]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[AuxVariables]
  [aux_var]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]


[ICs]
  [diffused_ic]
    type = MFEMScalarIC
    coefficient = -100.0
    variable = aux_var
  []
[]


[BCs]
  [bottom]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = 'bottom'
    coefficient = 0.0
  []
[]

[FunctorMaterials]
  [Substance]
    type = MFEMGenericFunctorMaterial
    prop_names = diffusivity
    prop_values = 1.0
    block = 'the_domain'
  []
[]

[Functions]
  [force]
    type = MFEMParsedFunction
    expression = 'aux_var*sin(10*z)'
    use_xyzt = true
   # symbol_names = 'c'
   # symbol_values = 'concentration'
    var_names = 'aux_var'
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = concentration
    coefficient = diffusivity
  []
  [source]
    type = MFEMDomainLFKernel
    variable = concentration
    coefficient = force
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [jacobi]
    type = MFEMOperatorJacobiSmoother
  []
[]

[Solver]
  type = MFEMHypreGMRES
  preconditioner = boomeramg
  l_tol = 1e-16
  l_max_its = 1000
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/DiffusionParsedSource
    vtk_format = ASCII
  []
[]
