# Complex (time-harmonic) analogue of subdomain_constraint_source.i. A complex
# scalar field is driven by a Dirichlet condition on the outer boundary and
# strongly constrained inside the "wire" subdomain to (2 + 3i) by an
# MFEMComplexScalarEssentialConstraint. With a purely real diffusion operator the
# real and imaginary parts decouple, so each is a harmonic extension of its
# boundary and subdomain values.

[Mesh]
  type = MFEMMesh
  file = ../mesh/hinomaru.e
[]

[Problem]
  type = MFEMProblem
  numeric_type = complex
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[Variables]
  [temperature]
    type = MFEMComplexVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [bc]
    type = MFEMComplexScalarDirichletBC
    variable = temperature
    boundary = outer
    coefficient_real = 1.0
    coefficient_imag = 0.0
  []
[]

[Constraints]
  [circle_interior]
    type = MFEMComplexScalarEssentialConstraint
    variable = temperature
    block = wire
    coefficient_real = 2.0
    coefficient_imag = 3.0
  []
[]

[Kernels]
  [diff]
    type = MFEMComplexKernel
    variable = temperature
    [RealComponent]
      type = MFEMDiffusionKernel
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

[VectorPostprocessors]
  [point_sample]
    type = MFEMComplexVariablePointValueSampler
    variable = 'temperature'
    points = '0.0 0.0 0.0  1.0 0.0 0.0  2.5 0.0 0.0'
  []
[]

[Outputs]
  csv = true
[]
