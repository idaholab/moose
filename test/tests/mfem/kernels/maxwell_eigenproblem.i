[Mesh]
  type = MFEMMesh
  file = ../mesh/beam-tet.mesh
  serial_refine = 2
  parallel_refine = 1
  dim = 3
[]

[Problem]
  type = MFEMEigenproblem
  num_modes = 5
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
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[BCs]
  [all]
    type = MFEMVectorTangentialDirichletBC
    variable = E
    vector_coefficient = '0 0 0'
  []
[]

[Kernels]
  [diff]
    type = MFEMCurlCurlKernel
    variable = E
  []
[]

[Preconditioner]
  [ams]
    type = MFEMHypreAMS
    fespace = HCurlFESpace
    print_level = 0
    singular = true
  []
[]

[Solver]
  type = MFEMHypreAME
  preconditioner = ams
  print_level = 0
  coefficient = 1.0
  l_tol = 1e-8
  l_max_its = 100
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [eigenvalues]
    type = MFEMEigenvaluesPostprocessor
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
  file_base = OutputData/MaxwellEigenproblem
[]
