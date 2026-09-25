# Exercises a single material declaring properties at different storage granularity: 'coarse' is
# constant over each subdomain and 'fine' varies per quadrature point.
#
# The framework evaluates the coarser granularity first, over one element per subdomain, so 'fine' can
# be computed from 'coarse'. The material sets coarse = 10 * (subdomain + 1) and fine = coarse + qp.
#
# The postprocessors read the properties directly rather than through auxiliary variables, so each is
# sampled at its own granularity. With two subdomains and four quadrature points per element the
# expected values follow from the material alone: coarse averages to 10 and 20, and fine averages to
# those plus the mean quadrature point index of 1.5.

[Mesh]
  [square]
    type = CartesianMeshGenerator
    dx = '5 5'
    ix = '5 5'
    dy = '5'
    iy = '5'
    dim = 2
    subdomain_id = '1 2'
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = KokkosMatDiffusionTest
    variable = u
    prop_name = fine
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = left
    value = 0.0
  []
  [right]
    type = KokkosDirichletBC
    variable = u
    boundary = right
    value = 1.0
  []
[]

[Materials]
  [mat]
    type = KokkosMixedGranularityTest
    coarse_name = 'coarse'
    fine_name = 'fine'
  []
[]

[Postprocessors]
  # Constant within each subdomain, and different between them
  [coarse_block1]
    type = KokkosElementAverageMaterialProperty
    mat_prop = 'coarse'
    block = 1
  []
  [coarse_block2]
    type = KokkosElementAverageMaterialProperty
    mat_prop = 'coarse'
    block = 2
  []
  # The same subdomain values plus the mean quadrature point index, which is what shows that the fine
  # pass read the coarse property and still varied per quadrature point
  [fine_block1]
    type = KokkosElementAverageMaterialProperty
    mat_prop = 'fine'
    block = 1
  []
  [fine_block2]
    type = KokkosElementAverageMaterialProperty
    mat_prop = 'fine'
    block = 2
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
[]
