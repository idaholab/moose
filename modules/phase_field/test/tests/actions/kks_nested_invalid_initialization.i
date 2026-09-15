[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Modules]
  [PhaseField]
    [KKS]
      phase_concentration_solve = NESTED
      phase_names = 'alpha beta gamma'
      order_parameters = 'eta_alpha eta_beta eta_gamma'
      global_concentrations = 'c b'
      free_energies = 'F_alpha F_beta F_gamma'
      switching_functions = 'h_alpha h_beta h_gamma'
      barrier_functions = 'g_alpha g_beta g_gamma'
      barrier_heights = '0 0 0'
      concentration_mobilities = 'M_c M_b'
      order_parameter_mobilities = 'L_alpha L_beta L_gamma'
      kappas = 'kappa_alpha kappa_beta kappa_gamma'
    []
  []
[]
