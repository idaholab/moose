# Time-harmonic (frequency-domain) magnetodynamic A-formulation solved with
# first-kind Nedelec elements. The complex magnetic vector potential A satisfies
#
#   curl(nu curl A) + i omega sigma A = J_src
#
# The i omega sigma A term removes the gradient null space of the curl-curl
# operator inside the conductor (block 1), but the surrounding insulator
# (block 2) has no such term and the discrete operator is singular there. A
# tree-cotree gauge restricted to block 2 fixes the real and imaginary parts of
# its edge DOFs on a spanning tree to zero, making the system solvable. Block 1
# edges seed the spanning forest but are not gauged.

conductor_block = '1'
insulator_block = '2'
reluctivity = 1.0
omega_sigma = 1.0 # angular frequency * conductivity

[Mesh]
  type = MFEMMesh
  file = ../mesh/waveguide.g
[]

[Problem]
  type = MFEMProblem
  numeric_type = complex
[]

[FESpaces]
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []
[]

[Variables]
  [a_field]
    type = MFEMComplexVariable
    fespace = HCurlFESpace
  []
[]

[Functions]
  [source_current]
    type = ParsedVectorFunction
    expression_x = '0'
    expression_y = '0'
    expression_z = '1'
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
    prop_names = 'omega_sigma'
    prop_values = ${omega_sigma}
    block = ${conductor_block}
  []
[]

[BCs]
  [tangential_a_zero]
    type = MFEMComplexVectorTangentialDirichletBC
    variable = a_field
    boundary = '1 2 3 4 5 6'
  []
[]

[Constraints]
  [tree_cotree_gauge]
    type = MFEMComplexTreeCotreeGaugeEssentialConstraint
    variable = a_field
    block = ${insulator_block}
    boundary = '1 2 3 4 5 6'
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMComplexKernel
    variable = a_field
    [RealComponent]
      type = MFEMCurlCurlKernel
      coefficient = reluctivity
    []
  []
  [eddy]
    type = MFEMComplexKernel
    variable = a_field
    block = ${conductor_block}
    [ImagComponent]
      type = MFEMVectorFEMassKernel
      coefficient = omega_sigma
    []
  []
  [source]
    type = MFEMComplexKernel
    variable = a_field
    block = ${conductor_block}
    [RealComponent]
      type = MFEMVectorFEDomainLFKernel
      vector_coefficient = source_current
    []
  []
[]

[Solvers]
  [main]
    type = MFEMMUMPS
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
  assembly_level = legacy
[]

[Postprocessors]
  # Time-averaged |A|^2 in the gauged insulator. This depends on the gauge (the
  # particular spanning tree), so it is only well defined once the gauge is
  # applied; with a canonical tree it is partition-independent.
  [InsulatorField]
    type = MFEMComplexVectorPeriodAveragedPostprocessor
    primal_variable = a_field
    dual_variable = a_field
    block = ${insulator_block}
  []
  # Time-averaged |A|^2 in the conductor: independent of the gauge (which does
  # not touch the conductor), a physics sanity check.
  [ConductorField]
    type = MFEMComplexVectorPeriodAveragedPostprocessor
    primal_variable = a_field
    dual_variable = a_field
    block = ${conductor_block}
  []
[]

[Outputs]
  csv = true
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/TreeCotreeGaugeTimeHarmonic
    vtk_format = ASCII
  []
[]
