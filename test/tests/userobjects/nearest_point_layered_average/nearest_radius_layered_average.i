# This input tests the NearestRadiusLayeredAverage object by taking the average
# of layered rings and using the variable u(x,y,x) = r + z, where r  sqrt(x^2 + y^2)
# Given a ring of inner and outer radii r1 and r2, respectively, and of height z1 and z2,
# the analytical solution is given by:
# avg(r1,r2,z1,z2) = 2/3 * (r1^2 + r1*r2 + r2^2) / (r1 + r2) + (z1 + z2) / 2
# Convergence to these values as num_sectors is increased is verified.

[Mesh]
  [./ccmg]
    type = ConcentricCircleMeshGenerator
    num_sectors = 8
    radii = '0.1 0.2 0.3 0.4 0.5'
    rings = '2 2 2 2 2'
    has_outer_square = false
    preserve_volumes = true
    smoothing_max_it = 3
  []
  [./extruder]
    type = MeshExtruderGenerator
    input = ccmg
    extrusion_vector = '0 0 1'
    num_layers = 4
  []
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./ring_average]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./reac]
    type = Reaction
    variable = u
  [../]
  [./forcing]
    type = BodyForce
    variable = u
    function = func
  [../]
[]

[Functions]
  [func]
    type = ParsedFunction
    expression = 'sqrt(x * x + y * y) + z'
  []
[]

[AuxKernels]
  [./np_layered_average]
    type = SpatialUserObjectAux
    variable = ring_average
    execute_on = timestep_end
    user_object = nrla
  [../]
[]

[UserObjects]
  [./nrla]
    type = NearestRadiusLayeredAverage
    direction = z
    num_layers = 2
    points = '0.05 0 0
              0.15 0 0
              0.25 0 0
              0.35 0 0
              0.45 0 0'
    variable = u
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
