# This simulation models the mechanics solution for a hollow sphere under
# pressure, applied on the outer surfaces, using 1D spherical symmetry
# assumpitions.  The inner radius of the sphere, r = 4mm, is pinned to prevent
# rigid body movement of the sphere.
#
# From Bower (Applied Mechanics of Solids, 2008, available online at
# solidmechanics.org/text/Chapter4_1/Chapter4_1.htm), and applying the outer
# pressure and pinned displacement boundary conditions set in this simulation,
# the radial displacement is given by:
#
# u(r) = \frac{P(1 + v)(1 - 2v)b^3}{E(b^3(1 + v) + 2a^3(1-2v))} * (\frac{a^3}{r^2} - r)
#
# where P is the applied pressure, b is the outer radius, a is the inner radius,
# v is Poisson's ration, E is Young's Modulus, and r is the radial position.
#
# The radial stress is given by:
#
# S(r) = \frac{Pb^3}{b^3(1 + v) + 2a^3(1 - 2v)} * (\frac{2a^3}{r^3}(2v - 1) - (1 + v))
#
# The test assumes an inner radius of 4mm, and outer radius of 9 mm,
# zero displacement at r = 4mm, and an applied outer pressure of 2MPa.
# The radial stress is largest in the inner most element and, at an assumed
# mid element coordinate of 4.5mm, is equal to -2.545MPa.
[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 4
  xmax = 9
  nx = 5
[]

[GlobalParams]
  displacements = 'disp_r'
[]

[Problem]
  coord_type = RSPHERICAL
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    add_variables = true
    use_automatic_differentiation = true
    spherical_center_point = '4.0 0.0 0.0'
    generate_output = 'spherical_radial_stress'
  []
[]

[Postprocessors]
  [stress_rr]
    type = ElementAverageValue
    variable = spherical_radial_stress
  []
[]

[BCs]
  [innerDisp]
    type = ADDirichletBC
    boundary = left
    variable = disp_r
    value = 0.0
  []
  [outerPressure]
    type = ADPressure
    boundary = right
    variable = disp_r
    factor = 2
  []
[]

[Materials]
  [Elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    poissons_ratio = 0.345
    youngs_modulus = 1e4
  []
  [stress]
    type = ADComputeFiniteStrainElasticStress
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  line_search = none

  # controls for linear iterations
  l_max_its = 100
  l_tol = 1e-8

  # controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-5

  # time control
  start_time = 0.0
  dt = 0.25
  dtmin = 0.0001
  end_time = 0.25
[]

[Outputs]
  exodus = true
[]
