[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3

  [./fmg]
    type = FileMeshGenerator
    file = 3D_cube.e
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
  [./disp_z]
  [../]
[]

[AuxVariables]
  [./gap_offset]
  [../]
  [./node_volume]
  [../]
[]

[AuxKernels]
  [./gap_offset]
    type = BoundaryOffsetPD
    variable = gap_offset
  [../]
  [./node_volume]
    type = NodalVolumePD
    variable = node_volume
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
  [./fix_z]
    type = DirichletBC
    variable = disp_z
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
