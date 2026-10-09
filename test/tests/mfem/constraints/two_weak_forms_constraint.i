# Two decoupled Poisson problems on the same mesh, each built by its own weak form. Both have
# constant Dirichlet data on the outer boundary and an essential constraint on the wire subdomain.
# Each constraint is named only by the weak form solving for its variable; the other weak form,
# which has no equation for that variable, would reject it. u2 is constrained to its boundary
# value, so it remains exactly 2, while u1 is constrained to 3 and departs from 1.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/hinomaru.e
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
    boundary = outer
    coefficient = 1.0
  []
  [u2_boundaries]
    type = MFEMScalarDirichletBC
    variable = u2
    boundary = outer
    coefficient = 2.0
  []
[]

[Constraints]
  [u1_wire]
    type = MFEMScalarEssentialConstraint
    variable = u1
    block = wire
    coefficient = 3.0
  []
  [u2_wire]
    type = MFEMScalarEssentialConstraint
    variable = u2
    block = wire
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
    constraints = 'u1_wire'
  []
  [SecondSystem]
    type = MFEMWeakForm
    kernels = 'diff_u2'
    bcs = 'u2_boundaries'
    constraints = 'u2_wire'
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
  csv = true
[]
