# Nonlinear heat conduction (MFEM Example 16), with the temperature-dependent diffusivity routed
# through a scalar quadrature function coefficient. The stored values are re-projected on each
# nonlinear iteration, reproducing the reference solve in nlheatconduction.i.

!include nlheatconduction.i

[Functions]
  [qf_k]
    type = MFEMScalarQuadratureFunction
    coefficient = diffusivity_temperature_dependence
    # the quadrature rule order matches the one used by DiffusionIntegrator for
    # second-order H1 elements on quadrilaterals (2 * fe_order + dim - 1 = 5)
    order = 5
  []
[]

[Kernels]
  active = 'qf_nl_diffusion linear_diffusion dT_dt'
  [qf_nl_diffusion]
    type = MFEMNLDiffusionKernel
    variable = temperature
    k_coefficient = qf_k
    dk_du_coefficient = ${alpha}
  []
[]
