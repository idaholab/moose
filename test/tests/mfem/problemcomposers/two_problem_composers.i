# Two decoupled Poisson problems on the same mesh, each with its own EquationSystem built by its
# own weak form and solved by its own ProblemOperator. Each problem has constant Dirichlet data on
# every boundary, so the exact solution of each is that constant: u1 == 1 and u2 == 2. The
# postprocessors therefore report zero error only if both operators solve their own system without
# disturbing the other.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/star.mesh
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
  [u1]
    type = MFEMVariable
    fespace = H1FESpace
  []
  [u2]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [u1_boundaries]
    type = MFEMScalarDirichletBC
    variable = u1
    boundary = '1'
    coefficient = 1.0
  []
  [u2_boundaries]
    type = MFEMScalarDirichletBC
    variable = u2
    boundary = '1'
    coefficient = 2.0
  []
[]

[Kernels]
  [diff_u1]
    type = MFEMDiffusionKernel
    variable = u1
  []
  [diff_u2]
    type = MFEMDiffusionKernel
    variable = u2
  []
[]

[WeakForms]
  [FirstSystem]
    type = MFEMWeakForm
    kernels = 'diff_u1'
    bcs = 'u1_boundaries'
  []
  [SecondSystem]
    type = MFEMWeakForm
    kernels = 'diff_u2'
    bcs = 'u2_boundaries'
  []
[]

[ProblemComposers]
  [FirstOperator]
    type = MFEMWeakFormProblemComposer
    weak_form = FirstSystem
  []
  [SecondOperator]
    type = MFEMWeakFormProblemComposer
    weak_form = SecondSystem
  []
[]

[Solvers]
  [main]
    type = MFEMMUMPS
    weak_form = FirstSystem
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Postprocessors]
  [u1_error]
    type = MFEML2Error
    variable = u1
    function = 1.0
  []
  [u2_error]
    type = MFEML2Error
    variable = u2
    function = 2.0
  []
[]

[Outputs]
  [TwoComposersCSV]
    type = CSV
    file_base = OutputData/TwoProblemComposers
  []
[]
