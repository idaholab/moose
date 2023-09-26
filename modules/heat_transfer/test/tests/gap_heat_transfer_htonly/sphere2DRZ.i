#
# 2DRZ Spherical Gap Heat Transfer Test.
#
# This test exercises 2D gap heat transfer for a constant conductivity gap.
#
# The mesh consists of an inner solid sphere of radius = 1 unit, and outer
# hollow sphere with an inner radius of 2. In other words, the gap between
# them is 1 radial unit in length.
#
# The conductivity of both spheres is set very large to achieve a uniform
# temperature in each sphere. The temperature of the center node of the
# inner sphere is ramped from 100 to 200 over one time unit. The
# temperature of the outside of the outer, hollow sphere is held fixed
# at 100.
#
# A simple analytical solution is possible for the integrated heat flux
# between the inner and outer spheres:
#
#  Integrated Flux = (T_left - T_right) * (gapK/(r^2*((1/r1)-(1/r2)))) * Area
#
# For gapK = 1 (default value)
#
# The area is taken as the area of the secondary (inner) surface:
#
# Area = 4 * pi * 1^2 (4*pi*r^2)
#
# The integrated heat flux across the gap at time 1 is then:
#
# 4*pi*k*delta_T/((1/r1)-(1/r2))
# 4*pi*1*100/((1/1) - (1/2)) =  2513.3 watts
#
# For comparison, see results from the integrated flux post processors.
# This simulation makes use of symmetry, so only 1/2 of the spheres is meshed
# As such, the integrated flux from the post processors is 1/2 of the total,
# or 1256.6 watts... i.e. 400*pi.
# The value coming from the post processor is slightly less than this
# but converges as mesh refinement increases.
#
# Simulating contact is challenging. Regression tests that exercise
# contact features can be difficult to solve consistently across multiple
# platforms. While designing these tests, we felt it worth while to note
# some aspects of these tests. The following applies to:
# sphere3D.i, sphere2DRZ.i, cyl2D.i, and cyl3D.i.
# 1. We decided that to perform consistently across multiple platforms we
# would use very small convergence tolerance. In this test we chose an
# nl_rel_tol of 1e-12.
# 2. Due to such a high value for thermal conductivity (used here so that the
# domains come to a uniform temperature) the integrated flux at time = 0
# was relatively large (the value coming from SideIntegralFlux =
#  -_diffusion_coef[_qp]*_grad_u[_qp]*_normals[_qp] where the diffusion coefficient
# here is thermal conductivity).
# Even though _grad_u[_qp] is small, in this case the diffusion coefficient
# is large. The result is a number that isn't exactly zero and tends to
# fail exodiff. For this reason the parameter execute_on = initial should not
# be used. That parameter is left to default settings in these regression tests.
#
[GlobalParams]
  order = SECOND
  family = LAGRANGE
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = cyl2D.e
[]

[Functions]
  [./temp]
    type = PiecewiseLinear
    x = '0   1'
    y = '100 200'
  [../]
[]


[Variables]
  [./temp]
    initial_condition = 100
  [../]
[]

[AuxVariables]
  [./gap_conductance]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]


[Kernels]
  [./heat_conduction]
    type = HeatConduction
    variable = temp
  [../]
[]


[AuxKernels]
  [./gap_cond]
    type = MaterialRealAux
    property = gap_conductance
    variable = gap_conductance
    boundary = 2
  [../]
[]

[Materials]
  [./heat1]
    type = HeatConductionMaterial
    block = '1 2'
    specific_heat = 1.0
    thermal_conductivity = 1000000.0
  [../]
[]

[ThermalContact]
  [./thermal_contact]
    type = GapHeatTransfer
    variable = temp
    primary = 3
    secondary = 2
    emissivity_primary = 0
    emissivity_secondary = 0
    gap_conductivity = 1
    quadrature = true
    gap_geometry_type = SPHERE
    sphere_origin = '0 0 0'
  [../]
[]

[BCs]
  [./mid]
    type = FunctionDirichletBC
    boundary = 1
    variable = temp
    function = temp
  [../]
  [./temp_far_right]
    type = DirichletBC
    boundary = 4
    variable = temp
    value = 100
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'

  dt = 1
  dtmin = 0.01
  end_time = 1

  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-7

  [./Quadrature]
    order = fifth
    side_order = seventh
  [../]
[]

[Outputs]
  exodus = true
  [./Console]
    type = Console
  [../]
[]

[Postprocessors]
  [./temp_left]
    type = SideAverageValue
    boundary = 2
    variable = temp
  [../]

  [./temp_right]
    type = SideAverageValue
    boundary = 3
    variable = temp
  [../]

  [./flux_left]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = 2
    diffusivity = thermal_conductivity
  [../]

  [./flux_right]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = 3
    diffusivity = thermal_conductivity
  [../]
[]
