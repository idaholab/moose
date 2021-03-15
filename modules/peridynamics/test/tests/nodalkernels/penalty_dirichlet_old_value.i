#In order to compare the solution generated using preset Dirichlet BC, the penalty was set to 1e10.
#Large penalty number should be used with caution.

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3

  [./gfm]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1.1
    xmax = 1.1
    ymin = -1.1
    ymax = 1.1
    nx = 11
    ny = 11
    elem_type = QUAD4
  [../]
  [./gpd]
    type = MeshGeneratorPD
    input = gfm
    retain_fe_mesh = false
  [../]
[]

[Variables]
  [./temp]
  [../]
[]

[AuxVariables]
  [./bond_status]
    initial_condition = 1
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConductionBPD
    variable = temp
  [../]
  [./source]
    type = HeatSourceBPD
    variable = temp
    power_density = '-4'
  [../]
[]

[Materials]
  [./thermal_material]
    type = ThermalConstantHorizonMaterialBPD
    thermal_conductivity = 1
    temperature = temp
  [../]
[]

[NodalKernels]
  [./bc_all]
    type = PenaltyDirichletOldValuePD
    variable = temp
    boundary = 'pd_nodes_top pd_nodes_left pd_nodes_right pd_nodes_bottom'
    penalty = 1e10
  [../]
[]

# [BCs]
#   [./fix]
#     type = DirichletBC
#     variable = temp
#     value = 0
#     boundary = 'pd_nodes_top pd_nodes_left pd_nodes_right pd_nodes_bottom'
#   [../]
# []

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  start_time = 0
  end_time = 1
  nl_rel_tol = 1e-14
[]

[Outputs]
  file_base = preset_bc_out
  exodus = true
[]
