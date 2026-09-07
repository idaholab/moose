# Solves the Poisson problem -Div(sigma Grad u) = 1, in 3D on a 2D surface,
# where the diffusion coefficient sigma is a 3x3 matrix.
# Based on MFEM Example ex29p. See corresponding `gold/` directory for more info.

[Mesh]
  type = MFEMMesh
  file = ../mesh/ex29p.mesh
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = 3
  []
[]

[Variables]
  [u]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [dirichlet]
    type = MFEMScalarDirichletBC
    variable = u
    coefficient = 0.0
  []
[]

[Kernels]
  [linear_form]
    type = MFEMDomainLFKernel
    variable = u
    coefficient = 1.0
  []
  [bilinear_form]
    type = MFEMCurlCurlKernel
    variable = u
    # ERROR: specifying both coefficient and matrix_coefficient.
    coefficient = 10.0
    matrix_coefficient = anisotropic_material
  []
[]

[FunctorMaterials]
  [anisotropic_material]
    type = MFEMGenericFunctorMatrixMaterial
    prop_names = anisotropic_material
    prop_values = '{ 100.0 1.5 0.0; 1.5 2.5 0.0; 0.0 0.0 3.0 }'
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Solvers]
  [main]
    type = MFEMCGSolver
    l_tol = 1e-12
    l_max_its = 2000
    use_initial_guess = false
  []
[]