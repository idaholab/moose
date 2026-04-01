# MFEM diffusion problem sampled with MFEMLineValueSampler.

[Mesh]
  type = MFEMMesh
  file = ../../mesh/mug.e
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

[ICs]
  [diffused_ic]
    type = MFEMScalarIC
    coefficient = one
    variable = concentration
  []
[]

[Functions]
  [one]
    type = ParsedFunction
    expression = 1.0
  []
[]

[BCs]
  [bottom]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = 'bottom'
    coefficient = 1.0
  []
  [top]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = 'top'
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

[VectorPostprocessors]
  [line_sample]
    type = MFEMLineValueSampler
    variable = 'concentration'
    start_point = '2.125 0 -2.375'
    end_point = '2.125 0 2.625'
    num_points = 11
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
[]
