# Elastic cube clamped on one face and loaded by the linear form (F, grad(v)) for a constant,
# non-symmetric tensor F. By the divergence theorem this equals the sum over the remaining faces of
# the tractions (F n, v), which the inactive boundary conditions below apply instead.

[Mesh]
  [cube]
    type = MFEMGeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
  []
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1VectorFESpace]
    type = MFEMVectorFESpace
    fec_type = H1
    fec_order = SECOND
    range_dim = 3
  []
[]

[Variables]
  [displacement]
    type = MFEMVariable
    fespace = H1VectorFESpace
  []
[]

[BCs]
  active = 'clamped'
  [clamped]
    type = MFEMVectorDirichletBC
    variable = displacement
    boundary = left
  []
  # Tractions F n, the columns of F signed by the outward normal of each face.
  [right_traction]
    type = MFEMVectorBoundaryIntegratedBC
    variable = displacement
    boundary = right
    vector_coefficient = '1 4 7'
  []
  [back_traction]
    type = MFEMVectorBoundaryIntegratedBC
    variable = displacement
    boundary = back
    vector_coefficient = '2 5 8'
  []
  [front_traction]
    type = MFEMVectorBoundaryIntegratedBC
    variable = displacement
    boundary = front
    vector_coefficient = '-2 -5 -8'
  []
  [top_traction]
    type = MFEMVectorBoundaryIntegratedBC
    variable = displacement
    boundary = top
    vector_coefficient = '3 6 10'
  []
  [bottom_traction]
    type = MFEMVectorBoundaryIntegratedBC
    variable = displacement
    boundary = bottom
    vector_coefficient = '-3 -6 -10'
  []
[]

[Kernels]
  [elasticity]
    type = MFEMLinearElasticityKernel
    variable = displacement
  []
  [tensor_source]
    type = MFEMVectorDomainLFGradKernel
    variable = displacement
    matrix_coefficient = '1 2 3; 4 5 6; 7 8 10'
  []
[]

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [main]
    type = MFEMHyprePCG
    preconditioner = boomeramg
    l_max_its = 1000
    l_tol = 1e-12
    l_abs_tol = 0.0
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [samples]
    type = MFEMVariablePointValueSampler
    variable = displacement
    points = '1 1 1
              1 0.5 0.5
              0.5 0 1'
  []
[]

[Outputs]
  [CSV]
    type = CSV
    file_base = OutputData/VectorDomainLFGrad
  []
[]
