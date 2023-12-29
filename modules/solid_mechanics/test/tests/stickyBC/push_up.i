# Testing StickyBC
#
# Push the bottom of an element upward until the top hits an (invisible) obstruction.
# 10 timesteps are used.  In each timestep disp_y is increased by 0.1.  The
# StickyBC has a max_value of 0.49, so at timestep 5 this bound will be violated
# and the top boundary will be fixed forever after.
#
# This test also illustrates that StickyBC is only ever meant to be used in
# special situations:
# - if, after the simulation ends, the bottom is moved downward again, the StickyBC
#   will keep the top fixed.  Ie, the StickyBC is truly "sticky".
# - setting max_value = 0.5 in this test illustrates the "approximate" nature
#   of StickyBC, in that some nodes will be fixed at disp_y=0.5, while others
#   will be fixed at disp_y=0.6, due to the timestepping and roundoff errors
#   in MOOSE's solution.
[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
  [../]
[]

[BCs]
  [./obstruction]
    type = StickyBC
    variable = disp_y
    boundary = top
    max_value = 0.49
  [../]
  [./bottom]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = bottom
    function = t
  [../]
  [./left]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./front]
    type = DirichletBC
    variable = disp_z
    boundary = front
    value = 0
  [../]
[]

[Materials]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1.0
    poissons_ratio = 0.2
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Linear
  dt = 0.1
  end_time = 1.0
[]

[Outputs]
  exodus = true
[]
