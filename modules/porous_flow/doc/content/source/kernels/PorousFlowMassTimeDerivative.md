# PorousFlowMassTimeDerivative

!syntax description /Kernels/PorousFlowMassTimeDerivative

This `Kernel` implements the weak form of
\begin{equation*}
  \frac{\partial}{\partial t}\left(\phi\sum_{\beta}S_{\beta}\rho_{\beta}\chi_{\beta}^{\kappa}\right)
\end{equation*}
where all parameters are defined in the [nomenclature](/nomenclature.md).

!alert note
A mass-lumped version is implemented.

See [mass lumping](/mass_lumping.md) for details.

The multiplication by $\rho_{\beta}$ is optional and is controlled by the `multiply_by_density` flag.  It is sometimes advantageous to use this flag because the problem becomes more linear.  However, this sometimes changes the nature of the physical problem modelled (consider, for instance, a fully-saturated multicomponent system, where `multiply_by_density = false` implies the steady-state Laplace equation for the porepressure), and care must be taken when using other PorousFlow objects (that intrinsically have `multiply_by_density = true`, such as [PorousFlowFluidMass](PorousFlowFluidMass.md)) so new users are encouraged to use the default `multiply_by_density = true` flag until they gain familiarity with PorousFlow.

For mechanically-coupled simulations (where the mesh deforms) the numerical implementation of this Kernel involves the old value of the volumetric strain.  Further information can be found [here](porous_flow/time_derivative.md).  This is assumed to be calculated by a TensorMechanics strain calculator, for instance [ComputeSmallStrain](ComputeSmallStrain.md), with a given `base_name`.  Hence, you should usually employ the same `base_name` for this Kernel as used by your strain calculator.  However, if you wish to include mechanical deformations, but not couple them to the porous flow, simply enter a `base_name` that doesn't exist (eg `base_name = non_existent`) and the volumetric strain won't be included in this Kernel.  (Note: if you want a fully-coupled simulation but accidentally make a typo in your `base_name`, then PorousFlow will assume you don't want to include the volumetric strain!)

Because it contains volumetric strain, this Kernel always sets `use_displaced_mesh = false` and the parameter cannot be altered by the user.  Further information can be found [here](porous_flow/time_derivative.md)

!syntax parameters /Kernels/PorousFlowMassTimeDerivative

!syntax inputs /Kernels/PorousFlowMassTimeDerivative

!syntax children /Kernels/PorousFlowMassTimeDerivative
