# This input file is used to test BoundaryFluxPostprocessor, which queries
# fluxes computed using user objects derived from BoundaryFluxBase. The boundary
# flux used in this test is TestBoundaryFlux, which expects a solution vector
# of size 3 (call this U = {A, B, C}) and computes a flux of size 2 with the
# following entries:
#
# flux[0] = (A - B) * C * nx
# flux[1] = A * B * nx
#
# where the normal vector used is {nx, ny, nz}.

A = 1
B = 2
C = 3

# Multiple cases are computed in this test. Each corresponds to a different PP object:
#   * flux0_boundary0: boundary 0, flux entry 0, default normal ({-1, 0, 0})
#   * flux0_boundary1: boundary 1, flux entry 0, default normal ({1, 0, 0})
#   * flux0_provided:  boundary 0, flux entry 0, user-provided normal ({2, 0, 0})
#   * flux1_boundary0: boundary 0, flux entry 1, default normal ({-1, 0, 0})

nx_boundary0 = -1
nx_boundary1 = 1
nx_provided  = 2

flux0_boundary0 = ${fparse (A - B) * C * nx_boundary0}
flux0_boundary1 = ${fparse (A - B) * C * nx_boundary1}
flux0_provided  = ${fparse (A - B) * C * nx_provided}
flux1_boundary0 = ${fparse A * B * nx_boundary0}

[GlobalParams]
  order = CONSTANT
  family = MONOMIAL
  execute_on = 'initial timestep_end'
  variables = 'A B C'
[]

[Postprocessors]
  [./flux0_boundary0]
    type = BoundaryFluxPostprocessor
    boundary_flux_uo = boundary_flux_flux0_boundary0
    boundary = 0
    flux_index = 0
  [../]
  [./flux0_boundary1]
    type = BoundaryFluxPostprocessor
    boundary_flux_uo = boundary_flux_flux0_boundary1
    boundary = 1
    flux_index = 0
  [../]
  [./flux0_provided]
    type = BoundaryFluxPostprocessor
    boundary_flux_uo = boundary_flux_flux0_provided
    boundary = 0
    flux_index = 0
    normal = '${nx_provided} 0 0'
  [../]
  [./flux1_boundary0]
    type = BoundaryFluxPostprocessor
    boundary_flux_uo = boundary_flux_flux1_boundary0
    boundary = 0
    flux_index = 1
  [../]

  [./flux0_boundary0_err]
    type = RelativeDifferencePostprocessor
    value1 = flux0_boundary0
    value2 = ${flux0_boundary0}
  [../]
  [./flux0_boundary1_err]
    type = RelativeDifferencePostprocessor
    value1 = flux0_boundary1
    value2 = ${flux0_boundary1}
  [../]
  [./flux0_provided_err]
    type = RelativeDifferencePostprocessor
    value1 = flux0_provided
    value2 = ${flux0_provided}
  [../]
  [./flux1_boundary0_err]
    type = RelativeDifferencePostprocessor
    value1 = flux1_boundary0
    value2 = ${flux1_boundary0}
  [../]
[]

[UserObjects]
  [./boundary_flux_flux0_boundary0]
    type = TestBoundaryFlux
  [../]
  [./boundary_flux_flux0_boundary1]
    type = TestBoundaryFlux
  [../]
  [./boundary_flux_flux0_provided]
    type = TestBoundaryFlux
  [../]
  [./boundary_flux_flux1_boundary0]
    type = TestBoundaryFlux
  [../]
[]

[Variables]
  [./A]
  [../]
  [./B]
  [../]
  [./C]
  [../]
[]

[ICs]
  [./A_ic]
    type = ConstantIC
    variable = A
    value = ${A}
  [../]
  [./B_ic]
    type = ConstantIC
    variable = B
    value = ${B}
  [../]
  [./C_ic]
    type = ConstantIC
    variable = C
    value = ${C}
  [../]
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Transient
  scheme = implicit-euler
  dt = 1
  num_steps = 1
[]

[Outputs]
  csv = true
  show = 'flux0_boundary0_err flux0_boundary1_err flux0_provided_err flux1_boundary0_err'
[]
