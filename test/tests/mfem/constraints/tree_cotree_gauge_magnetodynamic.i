# Magnetodynamic (eddy current) A-formulation solved with first-kind Nedelec
# elements. The magnetic vector potential A is only constrained up to a gradient,
# so the curl-curl operator has a large null space. Inside the conductor the
# sigma * dA/dt term removes it, but the surrounding insulator has no such term
# and the discrete operator is singular there. A tree-cotree gauge fixes the edge
# DOFs on a spanning tree of the insulator to zero, removing the residual null
# space and making the system solvable without an artificial mass regularisation.
#
# The mesh is a cylinder split into a conducting core (r <= 0.5) and an insulating
# shell (0.5 <= r <= 1). The shell is multiply connected - a loop encircling the
# core cannot be contracted within it - so the cotree carries a genuine loop and
# the gauge is not merely a spanning tree of a simply connected region.
#
#   nu       : reluctivity  (1 / magnetic permeability)   [whole domain]
#   sigma    : conductivity                               [conductor only]
#   J_src(t) : prescribed source current density          [conductor only]
#
# Weak form:  (nu curl A, curl v) + (sigma dA/dt, v) = (J_src, v)

conductor_block = 'interior'
insulator_block = 'exterior'
reluctivity = 1.0
conductivity = 1.0

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/cylinder-hex-q2.gen
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []
  [HDivFESpace]
    type = MFEMVectorFESpace
    fec_type = RT
    fec_order = CONSTANT
  []
[]

[Variables]
  [a_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[AuxVariables]
  [b_field]
    type = MFEMVariable
    fespace = HDivFESpace
  []
[]

[AuxKernels]
  [curl_a]
    type = MFEMCurlAux
    variable = b_field
    source = a_field
    scale_factor = 1.0
    execute_on = TIMESTEP_END
  []
[]

[Functions]
  # Azimuthal ramp current density, localised (by the source kernel) in the core.
  [source_current]
    type = ParsedVectorFunction
    expression_x = '-y * t'
    expression_y = ' x * t'
    expression_z = '0'
  []
[]

[FunctorMaterials]
  [Insulator]
    type = MFEMGenericFunctorMaterial
    prop_names = 'reluctivity'
    prop_values = ${reluctivity}
  []
  [Conductor]
    type = MFEMGenericFunctorMaterial
    prop_names = 'conductivity'
    prop_values = ${conductivity}
    block = ${conductor_block}
  []
[]

[BCs]
  [tangential_a_zero]
    type = MFEMVectorTangentialDirichletBC
    variable = a_field
    vector_coefficient = '0 0 0'
    boundary = 'front back curved_surface'
  []
[]

[Constraints]
  [tree_cotree_gauge]
    type = MFEMTreeCotreeGaugeEssentialConstraint
    variable = a_field
    # Gauge only the non-conducting region, where the sigma*dA/dt term is absent.
    # Conductor edges seed the spanning forest but are not gauged.
    block = ${insulator_block}
    # Boundaries where a tangential Dirichlet condition is applied to a_field, so
    # the interior gauge is seeded consistently with that boundary condition.
    boundary = 'front back curved_surface'
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMCurlCurlKernel
    variable = a_field
    coefficient = reluctivity
  []
  [eddy]
    type = MFEMTimeDerivativeVectorFEMassKernel
    variable = a_field
    coefficient = conductivity
    block = ${conductor_block}
  []
  [source]
    type = MFEMVectorFEDomainLFKernel
    variable = a_field
    vector_coefficient = source_current
    block = ${conductor_block}
  []
[]

[Solvers]
  [ams]
    type = MFEMHypreAMS
    fespace = HCurlFESpace
  []
  [main]
    type = MFEMHyprePCG
    preconditioner = ams
    l_tol = 1e-10
    l_max_its = 300
  []
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  dt = 0.25
  start_time = 0.0
  num_steps = 2
[]

[Postprocessors]
  # Magnetic field energy; a convenient deterministic scalar for regression.
  [MagneticEnergy]
    type = MFEMVectorFEInnerProductIntegralPostprocessor
    coefficient = reluctivity
    primal_variable = b_field
    dual_variable = b_field
  []
[]

[Outputs]
  csv = true
[]
