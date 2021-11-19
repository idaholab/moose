# A fracture, which is a 1D line of elements, is embedded in a matrix, which is a 2D surface of elements.
# The meshes conform: all fracture nodes are also matrix nodes (the fracture elements are sides of matrix elements).
# The overall mesh has two blocks, named "matrix" and "fracture".
#
# Two variables are defined:
# - frac_T, which is the temperature inside the fracture;
# - matrix_T, which is the temperature in the matrix.
# frac_T is governed by a diffusion equation along the 1D fracture.
# matrix_T is governed by a diffusion equation in the 2D matrix, with small diffusion coefficient.
# Heat is exchanged between the two systems via a heat-transfer coefficient, defined on the fracture subdomain, using two PorousFlowHeatMassTransfer Kernels
#
# If the mesh is too coarse, overshoots and undershoots in matrix_T can be observed.
[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    xmin = 0
    xmax = 10.0
    ny = 20 # anything less than this produces over/under-shoots
    ymin = -2
    ymax = 2
  []
  [matrix_subdomain]
    type = RenameBlockGenerator
    input = generate
    old_block = 0
    new_block = matrix
  []
  [fracture_sideset]
    type = ParsedGenerateSideset
    input = matrix_subdomain
    combinatorial_geometry = 'y>-1E-6 & y<1E-6'
    normal = '0 1 0'
    new_sideset_name = fracture_sideset
  []
  [fracture_subdomain]
    type = LowerDBlockFromSidesetGenerator
    input = fracture_sideset
    new_block_id = 1
    new_block_name = fracture
    sidesets = fracture_sideset
  []
[]

[Variables]
  [frac_T]
    block = fracture
  []
  [matrix_T]
    # Needs to be defined on both blocks, so PorousFlowHeatMassTransfer works appropriately
    # Kernels for diffusion are on block=matrix only
  []
[]

[BCs]
  [frac_T]
    type = DirichletBC
    variable = frac_T
    boundary = left
    value = 1
  []
[]

[Kernels]
  [dot_frac_T]
    type = CoefTimeDerivative
    Coefficient = 1E-2
    variable = frac_T
    block = fracture
  []
  [fracture_diffusion]
    type = AnisotropicDiffusion
    variable = frac_T
    tensor_coeff = '1E-2 0 0 0 1E-2 0 0 0 1E-2'
    block = fracture
  []
  [toMatrix]
    type = PorousFlowHeatMassTransfer
    block = fracture
    variable = frac_T
    v = matrix_T
    transfer_coefficient = 0.02
  []
  [dot_matrix_T]
    type = TimeDerivative
    variable = matrix_T
    block = matrix
  []
  [matrix_diffusion]
    type = AnisotropicDiffusion
    variable = matrix_T
    tensor_coeff = '1E-3 0 0 0 1E-3 0 0 0 1E-3'
    block = matrix
  []
  [fromFracture]
    type = PorousFlowHeatMassTransfer
    block = fracture
    variable = matrix_T
    v = frac_T
    transfer_coefficient = 0.02
  []
[]

[Preconditioning]
  [entire_jacobian]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 100
  end_time = 100
[]

[VectorPostprocessors]
  [frac_T]
    type = NodalValueSampler
    block = fracture
    outputs = frac_T
    sort_by = x
    variable = frac_T
  []
[]

[Outputs]
  print_linear_residuals = false
  exodus = false
  [frac_T]
    type = CSV
    execute_on = FINAL
  []
[]
