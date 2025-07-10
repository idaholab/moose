#This is a model of the dynamic response of a beam subjected
#to an axial pressure pulse applied to its end.

#This is a regression test intended to ensure that the
#Physics/SolidMechanics/Dynamic block can set the problem
#up correctly for a variety of 2D planar model options

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 0.1
  ymax = 1.0
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
#  out_of_plane_strain = 'strain_zz_var'
[]

[Variables]
#  [strain_zz_var]
#  []
[]

[Physics/SolidMechanics/Dynamic]
  [all]
    add_variables = true
    newmark_beta = 0.25
    newmark_gamma = 0.5
    strain = SMALL
    incremental = true
    density = 100
    generate_output = 'stress_yy strain_yy stress_zz strain_zz'
    #planar_formulation = PLANE_STRAIN #'WEAK_PLANE_STRESS'
  []
[]

[BCs]
  [top_x]
    type = DirichletBC
    variable = disp_x
    boundary = top
    value = 0.0
  []
  [top_y]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value = 0.0
  []
  [press_bot]
    type = Pressure
    variable = disp_y
    boundary = bottom
    function = 'if(t<0.5001,t*100,0)'
  []
[]

[Materials]
  [Elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1000
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 2
  dt = 0.1
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-20
[]

[Postprocessors]
  [disp_y_bot]
    type = NodalExtremeValue
    variable = disp_y
    boundary = bottom
  []
  [vel_y_bot]
    type = NodalExtremeValue
    variable = vel_y
    boundary = bottom
  []
  [accel_y_bot]
    type = NodalExtremeValue
    variable = accel_y
    boundary = bottom
  []
  [stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  []
  [strain_yy]
    type = ElementAverageValue
    variable = strain_yy
  []
  [stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  []
  [strain_zz]
    type = ElementAverageValue
    variable = strain_zz
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
