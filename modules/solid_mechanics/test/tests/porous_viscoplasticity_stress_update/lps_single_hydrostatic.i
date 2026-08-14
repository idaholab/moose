# This test provides an example of an individual LPS viscoplasticity model

!include lps_single.i

[BCs]
  inactive = 'pull_disp_y'
[]

[Materials]
  [lps]
    additional_porosity_pressure = 1e7
  []
  [porosity]
    initial_porosity := 1e-20
  []
[]

[Convergence]
  [disp_ref_check]
    nl_abs_tol = 1e-15
  []
[]
