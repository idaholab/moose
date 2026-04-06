# Integration test for MONOMIAL FIRST (order > 0) shape function evaluation
# through the Kokkos native FE (Phase 5) and on-demand FE (Phase 6) paths.
#
# A linear IC (x + y) is projected into a MONOMIAL FIRST aux variable.
# MONOMIAL FIRST can represent linear functions exactly, so the reconstructed
# values at quadrature points must match x + y.  The same input is run under
# three builds:
#   default            — libMesh FEBase phi tables
#   kokkos_native_fe   — CPU-native phi tables from initShapeNative (Phase 5)
#   kokkos_ondemand_fe — on-device nativeShape(FEShapeKey{MONOMIAL,...}) (Phase 6)
# All three must produce identical CSV output.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0
  xmax = 3.141
  ymin = 0
  ymax = 3.141
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [w]
    family = MONOMIAL
    order = FIRST
  []
  [w_copy]
    family = MONOMIAL
    order = FIRST
  []
[]

[ICs]
  [w_ic]
    type = FunctionIC
    variable = w
    function = 'x + y'
  []
[]

[AuxKernels]
  [copy_w]
    type = KokkosCopyValueAux
    variable = w_copy
    source = w
  []
[]

[VectorPostprocessors]
  [results]
    type = LineValueSampler
    start_point = '0.0001 0.99 0'
    end_point = '3.14 0.99 0'
    variable = 'w w_copy'
    num_points = 17
    sort_by = x
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
