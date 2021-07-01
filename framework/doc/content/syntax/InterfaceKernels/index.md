# InterfaceKernels System

Interface kernels are meant to assist in coupling different physics across sub-domains. The most straightforward example is the case in which one wants to set the flux of a specie A in subdomain 0 equal to the flux of a specie B in subdomain 1 at the boundary between subdomains 0 and 1. In mathematical terms, we might be interested in establishing the condition:

\begin{equation}
-D_0 \frac{\partial c_0}{\partial x} = -D_1 \frac{\partial c_1}{\partial x}
\end{equation}

where $D_i$ is the diffusion coefficient of specie $i$ in subdomain $i$, and $c_i$ is the concentration of specie $i$ in subdomain $i$. An example of this condition is shown in the MOOSE test directory; see files below:

[2d_interface/coupled_value_coupled_flux.i]

[/InterfaceDiffusion.C]

[/InterfaceDiffusion.h]

Interface kernels can be used to provide any general flux condition at an interface, and even more generally can be used to impose any interfacial condition that requires access to values of different variables and gradients of different variables on either side of an interface. In an input file, the user will specify at a minimum the following parameters:

- `type`: The type of interface kernel to be used
- `variable`: This is the "primary" variable. Note that the primary variable must exist on the same subdomain as the sideset specified in the `boundary` parameter. The existence of a "primary" and "secondary" or "neighbor" variable ensures that the interface kernel residual and jacobian functions get called the correct number of times. `variable` could be $c_0$ from our example above.
- `neighbor_var`: The "secondary" variable. This could be $c_1$ from our example above.
- `boundary`: The interfacial boundary between the subdomains. Note that this must be a sideset and again must exist on the same subdomain as the primary variable. The fact that this boundary is a sideset allows access to variable gradients.

An important private parameter relevant to displaced mesh calculations is
`_use_undisplaced_reference_points`. If an interface kernel developer sets this
parameter to `true` in the `validParams` of their derived class, then the
displaced problem will use the neighbor reference points computed by the
undisplaced problem as the neighbor reference points for the displaced
problem. If this parameter is `false` (the default), then the neighbor reference
points on the displaced problem will be computed in the traditional way: by
doing an inverse map of the physical locations of the displaced +element+
reference points. Here, reference refers to the reference coordinates
(e.g. $\xi$ and $\eta$) of the quadrature points and physical refers to the
physical coordinates (e.g. $x$ and $y$) of the quadrature points.

For additional information about the interface kernel system, don't hesitate to contact the [MOOSE Discussion forum](https://github.com/idaholab/moose/discussions).

!syntax list /InterfaceKernels objects=True actions=False subsystems=False

!syntax list /InterfaceKernels objects=False actions=False subsystems=True

!syntax list /InterfaceKernels objects=False actions=True subsystems=False
