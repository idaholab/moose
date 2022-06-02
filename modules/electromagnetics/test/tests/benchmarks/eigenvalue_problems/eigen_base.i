# Base input file for eigenvalue example tests for multiple waveguide geometries
# RECTANGULAR (Default)
#     Mesh file rectangular.e based on Mesh block:
#         [Mesh]
#           [gmg]
#             type = GeneratedMeshGenerator
#             dim = 2
#             nx = 50
#             ny = 25
#             xmin = 0
#             xmax = 2
#             ymin = 0
#             ymax = 1
#             elem_type = TRI3
#           []
#         []
#     Expected analytic eigenvalue = 12.337005
#     EM Module calculated eigenvalue = 12.363806
# CIRCULAR (Mesh/file=circle.msh, BCs/active='circle eigen_circle')
#     Mesh generated using gmsh
#       radius = 1
#       center = (0, 0)
#     Expected analytic eigenvalue = 5.784025
#     EM Module calculated eigenvalue = 5.824152
# COAXIAL (Mesh/file=coaxial.msh, BCs/active='coaxial eigen_coaxial')
#     Mesh generated using gmsh with coaxial.geo
#       inner_radius = 0.125
#       outer_radius = 0.5
#       center = (0, 0)
#     Expected analytic eigenvalue = 67.108864
#     EM Module calculated eigenvalue = 68.007802


[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = rectangular.e
  []
[]

[Variables]
  [potential]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [Ex]
    order = CONSTANT
    family = MONOMIAL
  []
  [Ey]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = potential
  []
  [coeff]
    type = CoefReaction
    coefficient = -1
    variable = potential
    extra_vector_tags = 'eigen'
  []
[]

[AuxKernels]
  [Ex_aux]
    type = PotentialToFieldAux
    variable = Ex
    gradient_variable = potential
    sign = negative
    component = x
  []
  [Ey_aux]
    type = PotentialToFieldAux
    variable = Ey
    gradient_variable = potential
    sign = negative
    component = y
  []
[]

[BCs]
  active = 'rectangle eigen_rectangle'
  [rectangle]
    type = DirichletBC
    variable = potential
    boundary = 'left right top bottom'
    value = 0
  []
  [eigen_rectangle]
    type = EigenDirichletBC
    variable = potential
    boundary = 'left right top bottom'
  []
  # alternative BCs for circle case
  [circle]
    type = DirichletBC
    variable = potential
    boundary = 'wall'
    value = 0
  []
  [eigen_circle]
    type = EigenDirichletBC
    variable = potential
    boundary = 'wall'
  []
  # alternative BCs for coaxial case
  [coaxial]
    type = DirichletBC
    variable = potential
    boundary = 'outer inner'
    value = 0
  []
  [eigen_coaxial]
    type = EigenDirichletBC
    variable = potential
    boundary = 'outer inner'
  []
[]

[VectorPostprocessors]
  [eigenvalues]
    type = Eigenvalues
  []
[]

[Executioner]
  type = Eigenvalue
[]

[Outputs]
  csv = true
  exodus = false
  execute_on = FINAL
[]
