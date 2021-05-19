#
# 2D Cylindrical Gap Heat Transfer Test.
#
# This test exercises 2D gap heat transfer for a constant conductivity gap.
#
# The mesh consists of an inner solid cylinder of radius = 1 unit, and outer
# hollow cylinder with an inner radius of 2 in the y-z plane. In other words,
# the gap between them is 1 radial unit in length.
#
# The calculated results are the same as for the cyl2D.i case in the x-y plane.

[GlobalParams]
  order = SECOND
  family = LAGRANGE
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = cyl2D.e
  []
  [./rotate]
    type = TransformGenerator
    transform = ROTATE
    vector_value = '0 90 90'
    input = file
  [../]
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
    gap_geometry_type = CYLINDER
    cylinder_axis_point_1 = '0 0 0'
    cylinder_axis_point_2 = '1 0 0'
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
