# Verification of Benchmark Problem TPV205-3D from the SCEC Dynamic Rupture Validation exercises #
# Reference: #
# Harris, R. M.-P.-A. (2009). The SCEC/USGS Dynamic Earthquake Rupture Code Verification Exercise. Seismological Research Letters, vol. 80, no. 1, pages 119-126. #

[Mesh]
    [./msh]
      type = GeneratedMeshGenerator
      dim = 3
      nx = 150
      ny = 150
      nz = 150
      xmin = -15000
      xmax = 15000
      ymin = -15000
      ymax = 15000
      zmin = -15000
      zmax = 15000
    []
    [./new_block]
      type = ParsedSubdomainMeshGenerator
      input = msh
      combinatorial_geometry = 'y<0'
      block_id = 1
    []
    [./split]
      type = BreakMeshByBlockGenerator
      input = new_block
      split_interface = true
    []
  []

  [GlobalParams]
    #primary variables
    displacements = 'disp_x disp_y disp_z'
    #damping ratio
    q = 0.1
    #characteristic length (m)
    Dc = 0.4
    #initial normal stress (Pa)
    T2_o = 120e6
    #initial shear stress along dip direction (Pa)
    T3_o = 0.0
    #dynamic friction coefficient
    mu_d = 0.525
    #element edge length (m)
    len = 200
  []

  [AuxVariables]
    [./resid_x]
        order = FIRST
        family = LAGRANGE
    [../]
    [./resid_y]
        order = FIRST
        family = LAGRANGE
    []
    [./resid_z]
        order = FIRST
        family = LAGRANGE
    []
    [./resid_slipweakening_x]
        order = FIRST
        family = LAGRANGE
    [../]
    [./resid_slipweakening_y]
        order = FIRST
        family = LAGRANGE
    [../]
    [./resid_slipweakening_z]
        order = FIRST
        family = LAGRANGE
    [../]
    [./disp_slipweakening_x]
        order = FIRST
        family = LAGRANGE
    []
    [./disp_slipweakening_y]
        order = FIRST
        family = LAGRANGE
    []
    [./disp_slipweakening_z]
        order = FIRST
        family = LAGRANGE
    []
    [./vel_slipweakening_x]
        order = FIRST
        family = LAGRANGE
    []
    [./vel_slipweakening_y]
        order = FIRST
        family = LAGRANGE
    []
    [./vel_slipweakening_z]
        order = FIRST
        family = LAGRANGE
    []
    [./mu_s]
        order = CONSTANT
        family = MONOMIAL
    []
    [./ini_shear_stress]
        order = CONSTANT
        family = MONOMIAL
    []
  []

  [Modules/TensorMechanics/CohesiveZoneMaster]
    [./czm_ik]
      boundary = 'Block0_Block1'
      strain = SMALL
    [../]
  []


  [Modules]
    [./TensorMechanics]
      [./Master]
        [./all]
          strain = SMALL
          add_variables = true
          extra_vector_tags = 'restore_tag'
        [../]
      [../]
    [../]
  []

  [Problem]
    extra_tag_vectors = 'restore_tag'
  []

  [AuxKernels]
    [Vel_x]
      type = ComputeValueRate
      variable = vel_slipweakening_x
      coupled = disp_x
      execute_on = 'TIMESTEP_BEGIN'
    []
    [Vel_y]
      type = ComputeValueRate
      variable = vel_slipweakening_y
      coupled = disp_y
      execute_on = 'TIMESTEP_BEGIN'
    []
    [Vel_z]
      type = ComputeValueRate
      variable = vel_slipweakening_z
      coupled = disp_z
      execute_on = 'TIMESTEP_BEGIN'
    []
    [Displacment_x]
      type = ProjectionAux
      variable = disp_slipweakening_x
      v = disp_x
      execute_on = 'TIMESTEP_BEGIN'
    []
    [Displacement_y]
      type = ProjectionAux
      variable = disp_slipweakening_y
      v = disp_y
      execute_on = 'TIMESTEP_BEGIN'
    []
    [Displacement_z]
      type = ProjectionAux
      variable = disp_slipweakening_z
      v = disp_z
      execute_on = 'TIMESTEP_BEGIN'
    []
    [Residual_x]
      type = ProjectionAux
      variable = resid_slipweakening_x
      v = resid_x
      execute_on = 'TIMESTEP_BEGIN'
    []
    [Residual_y]
      type = ProjectionAux
      variable = resid_slipweakening_y
      v = resid_y
      execute_on = 'TIMESTEP_BEGIN'
    []
    [Residual_z]
      type = ProjectionAux
      variable = resid_slipweakening_z
      v = resid_z
      execute_on = 'TIMESTEP_BEGIN'
    []
    [restore_x]
      type = TagVectorAux
      vector_tag = 'restore_tag'
      v = 'disp_x'
      variable = 'resid_x'
      execute_on = 'TIMESTEP_END'
    []
    [restore_y]
      type = TagVectorAux
      vector_tag = 'restore_tag'
      v = 'disp_y'
      variable = 'resid_y'
      execute_on = 'TIMESTEP_END'
    []
    [restore_z]
      type = TagVectorAux
      vector_tag = 'restore_tag'
      v = 'disp_z'
      variable = 'resid_z'
      execute_on = 'TIMESTEP_END'
    []
    [StaticFricCoeff]
      type = FunctionAux
      variable = mu_s
      function = func_static_friction_coeff_mus
      execute_on = 'LINEAR TIMESTEP_BEGIN'
    []
    [StrikeShearStress]
      type = FunctionAux
      variable = ini_shear_stress
      function = func_initial_strike_shear_stress
      execute_on = 'LINEAR TIMESTEP_BEGIN'
    []
  []

  [Kernels]
    [./inertia_x]
      type = InertialForce
      use_displaced_mesh = false
      variable = disp_x
    []
    [./inertia_y]
      type = InertialForce
      use_displaced_mesh = false
      variable = disp_y
    []
    [./inertia_z]
      type = InertialForce
      use_displaced_mesh = false
      variable = disp_z
    []
    [./Reactionx]
      type = StiffPropDamping
      variable = 'disp_x'
      component = '0'
    []
    [./Reactiony]
      type = StiffPropDamping
      variable = 'disp_y'
      component = '1'
    []
    [./Reactionz]
      type = StiffPropDamping
      variable = 'disp_z'
      component = '2'
    []
  []

  [Materials]
    [elasticity]
        type = ComputeIsotropicElasticityTensor
        lambda = 32.04e9
        shear_modulus = 32.04e9
        use_displaced_mesh = false
    []
    [stress]
        type = ComputeLinearElasticStress
    []
    [density]
        type = GenericConstantMaterial
        prop_names = density
        prop_values = 2670
    []
    [./czm_mat]
        type = SlipWeakeningFriction3d
        disp_slipweakening_x     = disp_slipweakening_x
        disp_slipweakening_y     = disp_slipweakening_y
        disp_slipweakening_z     = disp_slipweakening_z
        reaction_slipweakening_x = resid_slipweakening_x
        reaction_slipweakening_y = resid_slipweakening_y
        reaction_slipweakening_z = resid_slipweakening_z
        mu_s = mu_s
        ini_shear_sts = ini_shear_stress
        boundary = 'Block0_Block1'
    [../]
  []

  [Functions]
    [func_static_friction_coeff_mus]
      type = StaticFricCoeffMus
    []
    [func_initial_strike_shear_stress]
      type = InitialStrikeShearStress
    []
  []

  [UserObjects]
    [recompute_residual_tag]
        type = ResidualEvaluationUserObject
        vector_tag = 'restore_tag'
        force_preaux = true
        execute_on = 'TIMESTEP_END'
    []
  []

  [Executioner]
    type = Transient
    dt = 0.005
    end_time = 3.0
    [TimeIntegrator]
      type = CentralDifference
      solve_type = lumped
    []
  []

  [Outputs]
    exodus = true
    interval = 10
  []
