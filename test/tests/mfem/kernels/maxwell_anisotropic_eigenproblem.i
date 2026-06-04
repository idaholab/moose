# Anisotropic Maxwell eigenvalue problem, curl curl E = lambda epsilon E with
# E x n = 0, reproducing MFEM example ex32p. The anisotropy is carried by the
# 3x3 dielectric tensor epsilon on the right-hand-side mass matrix.

[Mesh]
  type = MFEMMesh
  file = ../mesh/beam-tet.mesh
  dim = 3
[]

[Problem]
  type = MFEMEigenproblem
  num_modes = 5
  rhs_coefficient = epsilon
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

[FunctorMaterials]
  [epsilon]
    type = MFEMGenericFunctorMatrixMaterial
    prop_names = epsilon
    prop_values = '{2 0.70710678118654752 0; 0.70710678118654752 2 0.70710678118654752; 0 0.70710678118654752 2}'
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
  file_base = OutputData/MaxwellAnisotropicEigenproblem
[]
