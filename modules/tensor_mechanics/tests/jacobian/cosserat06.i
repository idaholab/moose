[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  Cosserat_rotations = 'wc_x wc_y wc_z'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./wc_x]
  [../]
  [./wc_y]
  [../]
  [./wc_z]
  [../]
[]

[Kernels]
  active = 'cx_elastic cy_elastic cz_elastic x_couple y_couple z_couple x_moment y_moment z_moment'
  [./cx_elastic]
    type = CosseratStressDivergenceTensors
    variable = disp_x
    displacements = 'disp_x disp_y disp_z'
    component = 0
  [../]
  [./cy_elastic]
    type = CosseratStressDivergenceTensors
    variable = disp_y
    displacements = 'disp_x disp_y disp_z'
    component = 1
  [../]
  [./cz_elastic]
    type = CosseratStressDivergenceTensors
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
    component = 2
  [../]
  [./x_couple]
    type = StressDivergenceTensors
    variable = wc_x
    displacements = 'wc_x wc_y wc_z'
    component = 0
    base_name = couple
  [../]
  [./y_couple]
    type = StressDivergenceTensors
    variable = wc_y
    displacements = 'wc_x wc_y wc_z'
    component = 1
    base_name = couple
  [../]
  [./z_couple]
    type = StressDivergenceTensors
    variable = wc_z
    displacements = 'wc_x wc_y wc_z'
    component = 2
    base_name = couple
  [../]
  [./x_moment]
    type = MomentBalancing
    variable = wc_x
    component = 0
  [../]
  [./y_moment]
    type = MomentBalancing
    variable = wc_y
    component = 1
  [../]
  [./z_moment]
    type = MomentBalancing
    variable = wc_z
    component = 2
  [../]
[]


[Materials]
  [./elasticity_tensor]
    type = ComputeCosseratElasticityTensor
    B_ijkl = '1111 1112 1113 1121 1122 1123 1131 1132 1133   1112 1212 1213 1221 1222 1223 1231 1232 1233    1113 1213 1313 1321 1322 1323 1331 1332 1333     1121 1221 1321 2121 2122 2123 2131 2132 2133     1122 1222 1322 2122 2222 2223 2231 2232 2233     1123 1223 1323 2123 2223 2323 2331 2332 2333     1131 1231 1331 2131 2231 2331 3131 3132 3133     1132 1232 1332 2132 2232 2332 3132 3232 3233     1133 1233 1333 2133 2233 2333 3133 3233 3333'
    fill_method_bending = 'general'
    E_ijkl = '1111 1112 1113 1121 1122 1123 1131 1132 1133   1112 1212 1213 1221 1222 1223 1231 1232 1233    1113 1213 1313 1321 1322 1323 1331 1332 1333     1121 1221 1321 2121 2122 2123 2131 2132 2133     1122 1222 1322 2122 2222 2223 2231 2232 2233     1123 1223 1323 2123 2223 2323 2331 2332 2333     1131 1231 1331 2131 2231 2331 3131 3132 3133     1132 1232 1332 2132 2232 2332 3132 3232 3233     1133 1233 1333 2133 2233 2333 3133 3233 3333'
    fill_method = 'general'
  [../]
  [./strain]
    type = ComputeCosseratSmallStrain
  [../]
  [./stress]
    type = ComputeCosseratLinearElasticStress
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
[]
