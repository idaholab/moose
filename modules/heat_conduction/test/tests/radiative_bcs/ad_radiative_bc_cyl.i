#
# Thin cylindrical shell with very high thermal conductivity
# so that temperature is almost uniform at 500 K. Radiative
# boundary conditions is applied. Heat flux out of boundary
# 'right' should be 3723.36; this is approached as the mesh
# is refined
#

[Mesh]
  type = MeshGeneratorMesh
  [cartesian]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    ix = '1 10'
    dy = '1 1'
    subdomain_id = '1 2 1 2'
  []

  [remove_1]
    type = BlockDeletionGenerator
    block = 1
    input = cartesian
  []

  [readd_left]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'abs(x - 1) < 1e-4'
    new_sideset_name = left
    input = remove_1
  []
[]

[Problem]
  coord_type = RZ
[]

[Variables]
  [temp]
    initial_condition = 800.0
  []
[]

[Kernels]
  [heat]
    type = ADHeatConduction
    variable = temp
  []
[]

[BCs]
  [lefttemp]
    type = ADDirichletBC
    boundary = left
    variable = temp
    value = 800
  []

  [radiative_bc]
    type = ADInfiniteCylinderRadiativeBC
    boundary = right
    variable = temp
    boundary_radius = 2
    boundary_emissivity = 0.2
    cylinder_radius = 3
    cylinder_emissivity = 0.7
    Tinfinity = 500
  []
[]

[Materials]
  [density]
    type = ADGenericConstantMaterial
    prop_names = 'density  thermal_conductivity'
    prop_values = '1 1.0e5'
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_converged_reason'
  line_search = none
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-7
[]

[Postprocessors]
  [right]
    type = ADSideDiffusiveFluxAverage
    variable = temp
    boundary = right
    diffusivity = thermal_conductivity
  []

  [min_temp]
    type = ElementExtremeValue
    variable = temp
    value_type = min
  []

  [max_temp]
    type = ElementExtremeValue
    variable = temp
    value_type = max
  []
[]

[Outputs]
  csv = true
[]
