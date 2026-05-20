# VariableResidualNormBase

This Postprocessor base class is used for computing a norm of the nonlinear residual
vector $\mathbf{r}$ for a variable $u$ over a chosen domain $\Omega$.

The parameter [!param](/Postprocessors/DiscreteVariableResidualNorm/include_scaling_factor)
controls whether the residual/Jacobian scaling factor (associated with the selected solution variable)
should be included in the norm. For the purposes of convergence monitoring, it is usually
recommended to keep this as `false`, as residual/Jacobian scaling factors are usually intended to improve
numerical conditioning of the system rather than convergence criteria, although in some instances a user may deliberately be selecting scaling factors to influence residual-based convergence using the `Executioner/resid_vs_jac_scaling_param`; in this case the user likely wishes to set `include_scaling_factor = true`.

Note the related post-processors:

- [VariableResidual.md]: computes the $\ell_2$ norm of the nonlinear residual norm for a specific variable $u$ over the domain of $u$, $\Omega_u$.
- [/Residual.md]: computes the $\ell_2$ norm of the nonlinear residual norm for all variables over the entire domain $\Omega_\text{total}$.
