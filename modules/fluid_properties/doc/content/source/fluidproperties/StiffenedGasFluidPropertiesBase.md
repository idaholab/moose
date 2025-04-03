# StiffenedGasFluidPropertiesBase

This is a base class for [SinglePhaseFluidProperties.md] objects that use the
stiffened equation of state [!citep](metayer2004):

!equation
p = (\gamma - 1) \rho (e - q) - \gamma p_{\infty} \,,

where $\gamma = c_p/c_v$ is the ratio of specific heat capacities, $q$ is a constant that defines the
zero reference state for internal energy, and $p_{\infty}$ is a constant representing the attraction
between fluid molecules that makes the fluid *stiff* in comparison to an ideal gas. This equation of
state is typically used to represent water that is under very high pressure.

!bibtex bibliography
