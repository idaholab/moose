[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3

  [./fmg]
    type = FileMeshGenerator
    file = 2D_square.e
  [../]
  [./mgpd]
    type = MeshGeneratorPD
    input = fmg
    retain_fe_mesh = false
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./gap_offset]
  [../]
  [./node_area]
  [../]
[]

[AuxKernels]
  [./gap_offset]
    type = BoundaryOffsetPD
    variable = gap_offset
  [../]
  [./node_area]
    type = NodalVolumePD
    variable = node_area
  [../]
[]

[Modules/Peridynamics/Mechanics/Master]
  [./blk1]
    formulation = BOND
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]

  [./material_pd]
    type = ComputeSmallStrainVariableHorizonMaterialBPD
  [../]
[]

[BCs]
  [./fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1001
    value = 0
  [../]
  [./fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1001
    value = 0
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  end_time = 1
[]

[Outputs]
  exodus = true
[]
