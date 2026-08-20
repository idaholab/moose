freq = 5.0
omega = ${fparse 2 * pi * freq}
epsilon = 1.0
mu = 1.0
reluctivity = ${fparse 1.0 / mu}
mass_coefficient = ${fparse -omega * omega * epsilon}
decay_coefficient = ${fparse 5.0 / (omega * sqrt(epsilon * mu))}
source_width = ${fparse 5 * omega * sqrt(epsilon * mu) / pi}
source_width_squared = ${fparse source_width * source_width}
source_amplitude = ${fparse source_width_squared / pi}
z_center = 0

[Mesh]
  type = MFEMMesh
  file = ../mesh/inline-quad.mesh
  uniform_refine = 3
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
  [E]
    type = MFEMComplexVariable
    fespace = HCurlFESpace
  []
[]

[Functions]
  [source_field]
    type = ParsedVectorFunction
    expression_x = '${source_amplitude} * exp(-${source_width_squared} * ((x - 0.5)^2 + (y - 0.5)^2 + (z - ${z_center})^2))'
    expression_y = '0'
  []
[]

[SubMeshes]
  [pml]
    type = MFEMTransitionSubMesh
    boundary = '1 2 3 4'
    block = 1
    transition_subdomain = pml_dom
    transition_subdomain_boundary = pml_inner
    closed_subdomain = full_dom
    num_layers_positive = 8
  []
[]

[Kernels]
  [curlcurl_interior]
    type = MFEMComplexKernel
    variable = E
    block = 1
    [RealComponent]
      type = MFEMCurlCurlKernel
      coefficient = ${reluctivity}
    []
  []
  [mass_interior]
    type = MFEMComplexKernel
    variable = E
    block = 1
    [RealComponent]
      type = MFEMVectorFEMassKernel
      coefficient = ${mass_coefficient}
    []
  []
  [curlcurl_pml]
    type = MFEMPMLCurlCurlKernel
    variable = E
    block = pml_dom
    coefficient = ${reluctivity}
    decay_coefficient = ${decay_coefficient}
    decay_polynomial = 2
  []
  [mass_pml]
    type = MFEMPMLVectorFEMassKernel
    variable = E
    block = pml_dom
    coefficient = ${mass_coefficient}
    decay_coefficient = ${decay_coefficient}
    decay_polynomial = 2
  []
  [source]
    type = MFEMComplexKernel
    variable = E
    [ImagComponent]
      type = MFEMVectorFEDomainLFKernel
      vector_coefficient = source_field
    []
  []
[]

[BCs]
  [tangential_E]
    type = MFEMComplexVectorTangentialDirichletBC
    variable = E
    boundary = '1 2 3 4'
    vector_coefficient_real = '0. 0.'
    vector_coefficient_imag = '0. 0.'
  []
[]

[Solvers]
  [main]
    type = MFEMSuperLU
  []
[]

[Executioner]
  type = MFEMSteady
  assembly_level = legacy
[]

[Postprocessors]
  [interior_energy]
    type = MFEMComplexVectorPeriodAveragedPostprocessor
    primal_variable = E
    dual_variable = E
    block = 1
  []
[]

[Outputs]
  [ReportedPostprocessors]
    type = CSV
    file_base = OutputData/PML
  []
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/PML
    vtk_format = ASCII
  []

[]
