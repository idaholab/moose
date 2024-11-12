[Mesh]
  type = MFEMMesh
  file = mug.e
  dim = 3
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[Variables]
  [diffused0]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]
[AuxVariables]
  [sub0]
    type = MFEMVariable
    fespace = H1FESpace
    ic = -100.0
  []
[]

[Functions]
  [value_bottom]
    type = ParsedFunction
    expression = 1.0
  []
  [value_top]
    type = ParsedFunction
    expression = 0.0
  []
[]

[BCs]
  [bottom]
    type = MFEMScalarDirichletBC
    variable = diffused0
    boundary = '1'
    coefficient = BottomValue
  []
  [low_terminal]
    type = MFEMScalarDirichletBC
    variable = diffused0
    boundary = '2'
    coefficient = TopValue
  []
[]

[Materials]
  [Substance]
    type = MFEMGenericConstantMaterial
    prop_names = diffusivity
    prop_values = 1.0
  []
[]

[Coefficients]
  [TopValue]
    type = MFEMFunctionCoefficient
    function = value_top
  []
  [BottomValue]
    type = MFEMFunctionCoefficient
    function = value_bottom
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = diffused0
    coefficient = diffusivity
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
  l_max_its = 1000  
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/Diffuseon
    vtk_format = ASCII
  []
[]

[MultiApps]
  [./subapp]
    type = FullSolveMultiApp
    input_files = sub.i
    execute_on = FINAL 
  [../]
[]

[Transfers]
    [./to_sub]
        type = MFEMCopyTransfer
        source_variable = diffused0
        variable = sub1 
        #execute_on = FINAL
        to_multi_app = subapp
        #skip_coordinate_collapsing = true
    [../]
[]

#[Transfers]
#  [./to_sub]
#    type = MultiAppCopyTransfer
#    source_variable = u
#    variable = u
#    to_multi_app = sub
#  [../]
#[]

