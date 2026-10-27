# Exercises the PointInSubdomainCheckUO point-query accessors directly.
#
# Uses the same five-grain boundary mesh as grain_marking.i, but instead of
# reassigning element subdomains with SubdomainElementModifier, it evaluates the
# in-out test at each background element centroid:
#   - 'which_sub' records whichSubdomain(centroid): the grain (subdomain) ID that
#     contains the centroid, or the invalid-subdomain sentinel if none does.
#   - 'inside' records ifInside(centroid): 1 if the centroid lies in any grain,
#     0 otherwise.
# The resulting elemental fields are written to Exodus for comparison.

[Problem]
  solve = false
[]

[Mesh]
  [boundary_mesh]
    type = FileMeshGenerator
    file = 'grain_5_boundary.msh'
    save_with_name = 'boundary_mesh'
  []
  # Deliberately larger than the grain region [-0.6, 0.6] x [-0.4, 0.4] so that
  # border elements fall outside every grain. This exercises both the matched
  # branch (a grain contains the centroid) and the exhausted-loop branch
  # (no grain does) of whichSubdomain() and ifInside().
  [background_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -0.8
    xmax = 0.8
    ymin = -0.55
    ymax = 0.55
    nx = 10
    ny = 8
    subdomain_ids = 0
  []
  final_generator = background_mesh
[]

[UserObjects]
  [surface_builder]
    type = SurfaceMeshBySubdomainBuilder
    surface_mesh = boundary_mesh
  []
  [subdomain_tester]
    type = PointInSubdomainCheckUO
    builder = surface_builder
  []
[]

[AuxVariables]
  [which_sub]
    family = MONOMIAL
    order = CONSTANT
  []
  [inside]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [which_sub]
    type = PointInSubdomainTestAux
    variable = which_sub
    subdomain_checker = subdomain_tester
    method = which_subdomain
    execute_on = INITIAL
  []
  [inside]
    type = PointInSubdomainTestAux
    variable = inside
    subdomain_checker = subdomain_tester
    method = if_inside
    execute_on = INITIAL
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
