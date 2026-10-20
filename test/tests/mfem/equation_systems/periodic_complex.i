# Complex screened Poisson problem -div(grad u) + (1 + i) u = f on a periodic square with no
# boundary elements, with exact solution u = sin(pi x) sin(pi y).

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/periodic-square.mesh
  serial_refine = 2
[]

[Problem]
  type = MFEMProblem
  numeric_type = complex
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = SECOND
  []
[]

[Variables]
  [u]
    type = MFEMComplexVariable
    fespace = H1FESpace
  []
[]

[Functions]
  [source_real]
    type = ParsedFunction
    expression = '(1 + 2 * pi^2) * sin(pi * x) * sin(pi * y)'
  []
  [source_imag]
    type = ParsedFunction
    expression = 'sin(pi * x) * sin(pi * y)'
  []
  [exact]
    type = ParsedFunction
    expression = 'sin(pi * x) * sin(pi * y)'
  []
[]

[Kernels]
  [diffusion]
    type = MFEMComplexKernel
    variable = u
    [RealComponent]
      type = MFEMDiffusionKernel
    []
    [ImagComponent]
      type = MFEMDiffusionKernel
      coefficient = 0.0
    []
  []
  [mass]
    type = MFEMComplexKernel
    variable = u
    [RealComponent]
      type = MFEMMassKernel
    []
    [ImagComponent]
      type = MFEMMassKernel
    []
  []
  [source]
    type = MFEMComplexKernel
    variable = u
    [RealComponent]
      type = MFEMDomainLFKernel
      coefficient = source_real
    []
    [ImagComponent]
      type = MFEMDomainLFKernel
      coefficient = source_imag
    []
  []
[]

[Solvers]
  [main]
    type = MFEMSuperLU
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Postprocessors]
  [l2_error]
    type = MFEMComplexL2Error
    variable = u
    function_real = exact
    function_imag = 0
  []
[]

[Outputs]
  [CSV]
    type = CSV
    file_base = OutputData/PeriodicComplex
  []
[]
