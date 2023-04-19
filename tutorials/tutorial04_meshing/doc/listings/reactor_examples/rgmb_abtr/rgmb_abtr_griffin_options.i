[Mesh]
  [mesh]
    type = CoarseMeshExtraElementIDGenerator
    input = abtr_mesh
    coarse_mesh = abtr_mesh
    extra_element_id_name = coarse_element_id
  []
[]

[TransportSystems]
  particle = neutron
  G = 33
  VacuumBoundary = 'outer_core top bottom'
  equation_type = eigenvalue
  [sn]
    scheme = DFEM-SN
    family = L2_LAGRANGE
    order = FIRST
    AQtype = Gauss-Chebyshev
    NPolar = 3
    NAzmthl = 4
    NA = 2
    sweep_type = asynchronous_parallel_sweeper
    using_array_variable = true
    collapse_scattering  = true
  []
[]

[Executioner]
  type = SweepUpdate
  verbose = true

  richardson_value = fission_source_integral
  richardson_abs_tol = 1e-6
  richardson_max_its = 500
  inner_solve_type = GMRes
  max_inner_its = 8
  debug_richardson = true

  cmfd_acceleration = false
  coarse_element_id = coarse_element_id
  diffusion_eigen_solver_type = krylovshur
  max_diffusion_coefficient = 10
[]

[GlobalParams]
  library_file = mcc3.abtr.33macro3d_rgmb.xml
  library_name =  ISOTXS-neutron
  densities = 1.0
  plus = true
  dbgmat = false
  grid_names = 'Tfuel'
  grid = '1'
[]

[Materials]
  [matid]
    type = MixedMatIDNeutronicsMaterial
    block = 'RGMB_CORE'
    isotopes = 'pseudo'
  []
[]

[Outputs]
  csv = true
  [console]
    type = Console
    outlier_variable_norms = false
  []
  [pgraph]
    type = PerfGraphOutput
    level = 2
  []
[]
