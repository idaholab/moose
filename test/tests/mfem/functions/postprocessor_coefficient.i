# Checks that the value of a postprocessor is available as a spatially uniform scalar coefficient
# named after it. The postprocessor used here is a ConstantPostprocessor, which is not an MFEM
# object, so this covers values calculated outside the MFEM problem, such as ones transferred in
# from a subapp.
#
# The problem solved is the mass matrix system (u, v) = (k, v), whose solution is the constant
# field u = k, exactly representable in the H1 space used. Comparing against that constant checks
# that the coefficient takes the value the postprocessor holds and not, say, the zero it holds
# before it is first executed.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/ref-cube.mesh
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
  [u]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[Postprocessors]
  # Coefficients built from postprocessor values are not ordered against the postprocessors
  # supplying them, so the supplying postprocessor is executed on an earlier flag than the objects
  # consuming the coefficient, here the equation system assembled for the solve.
  [Amplitude]
    type = ConstantPostprocessor
    value = 2.5
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [SolutionError]
    type = MFEML2Error
    variable = u
    function = expected_solution
  []
[]

[Functions]
  [expected_solution]
    type = ParsedFunction
    expression = 2.5
  []
[]

[Kernels]
  [mass]
    type = MFEMMassKernel
    variable = u
  []
  # Amplitude here is the coefficient named after the postprocessor supplying its value.
  [source]
    type = MFEMDomainLFKernel
    variable = u
    coefficient = Amplitude
  []
[]

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [main]
    type = MFEMHyprePCG
    preconditioner = boomeramg
    l_tol = 1e-16
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  csv = true
  file_base = OutputData/PostprocessorCoefficient
[]
