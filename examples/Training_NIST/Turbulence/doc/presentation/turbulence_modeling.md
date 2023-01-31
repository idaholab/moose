# Brief discussion about turbulence modeling in CFD

!---

## RANS Navier-Stokes equations

The RANS Navier-Stokes equations system with heat transfer solved in MOOSE for the ```weakly-compressible``` formulation reads as follows:

$\frac{\partial \rho(T)}{\partial t} + \nabla \cdot \left( \rho(T) \vec{u} \right)$

$\frac{\partial \rho(T) \vec{u}}{\partial t} + \nabla \cdot \left( \rho(T) \vec{u} \vec(u) \right) = -\nabla p + \nabla \cdot \left( (\mu(T) + \mu_t) (\nabla \vec{u} + (\nabla \vec{u})^T) \right) + \rho \vec{g}$

$\frac{\partial \rho C_p \rho(T)}{\partial t} + \nabla \cdot \left( \rho(T) \vec{u} C_p T\right) = \nabla \cdot \left( (k(T) + k_t) \nabla T \right) + q$

With: $k_t = \frac{\mu_t C_p}{Pr_t}$ and $Pr_t \approx 0.9$

The turbulence modeling problem is finding a good value for $\mu_t=\mu_t(\vec{u},\rho,\mu)$

!---

## The mixing length model

!row!
!col! width=65%

!style! fontsize=80%
- The mixing length model is a method attempting to describe momentum transfer by turbulence Reynolds stresses within fluid boundary layer by means of an eddy viscosity.

- The mixing length is conceptually analogous to the concept of mean free path in thermodynamics: a fluid parcel will conserve its properties for a characteristic length $\ell_m$

- The Boussinesq hypothesis for a boundary layer with streamwise velocity $u$ and perpendicular $v$ states that the turbulent transport in the direction perpendicular to the wall can be approximated by $\overline{u'v'} = \frac{\mu_t}{\rho} \frac{\partial u}{\partial y}$

- For the turbulent diffusivity, $\nu_t=\frac{\mu_t}{\rho}$, and using unit analysis, Prandtl assumed that that this one could be approximated by $\nu_t = U \ell_m$, where $U$ is a characterisitc mixing velocity

- The characteristic mixing velocity can be further approximated by the extend to which turbulent gradients can "mix" the flow, i.e., $U = \ell_m \left| \frac{\partial u}{\partial y} \right|$

- This yields the so called mixing model, which approximates $\overline{u'v'} = \ell_m^2 \left| \frac{\partial u}{\partial y} \right| \frac{\partial u}{\partial y}$ and hence, $\mu_t = \rho \ell_m^2 \left| \frac{\partial u}{\partial y} \right|$

!style-end!

!col-end!

!col! width=35%
!media mixing_length_example.png style=margin-left:auto;margin-right:auto;display:block;
!col-end!

!row-end!

!---

## The mixing length model (Cntd.)

!row!
!col! width=65%

!style! fontsize=80%

- Although approximates, the mixing length theory turns out to be surprisingly accurate for boundary layer development flows

- Different studies have related the mixing length $\ell_m$ to the characterisitc dimmension of free shear flows $\ell$ as follows:

  - $\ell_m = 0.09 \ell$ for plane jets
  -  $\ell_m = 0.075 \ell$ for a circular jet
  -  $\ell_m = 0.07 \ell$ for a mixing layer

- Additionally, for boundary layer flows, Escudier proposed the model currently implemented in ```MOOSE-FV```:

  - If $\frac{y}{\delta} \leq \frac{\kappa_0}{\kappa}$: $\ell_m = \kappa y$
  - If $\frac{y}{\delta} ? \frac{\kappa_0}{\kappa}$: $\ell_m = \kappa_0 \delta$

where $\kappa=0.41$ is the Von Karman constant (meassured for boundary layer flows), $\kappa_0$ is a capping constant callibrated by Escudier, $y$ is the distance from the wall, and $\delta$ is a user specified parameter regarding the size of the domain.

!style-end!

!col-end!

!col! width=35%
!media escudier_model.png style=margin-left:auto;margin-right:auto;display:block;
!col-end!

!row-end!

!---

## Intuition behind the mixing length model

- As flow develops over a boundary layer and the flow preserves 2D-like characteristic, we know that $\overline{u'v'} = \kappa^2 y^2 \left| \frac{\partial u}{\partial y} \right|^2$ and hence $\ell_m = \kappa y$. Boundary layer turbulence theory is one of the few things that we know about turbulence.

- So, as an analyst, our job is determining where the hypothesis of von Karman mixing ends. Be careful because in a 3D problem, the dimension of mixing-hypothsis break must be estimated in the three dimensions simultaneously.

- A few tips:

  - If we expect the mixing hypothesis to be valid all across a direction $i$, the peak $\delta > \ell$ with $\ell$ being the domain dimension in direction $i$
  - If we expect the mixing length hypothesis to break but we are not sure where, $\delta \approx \ell/2$ is often a good hypothesis
  - If higher accuracy is needed, evaluate using two-equation models or find experimental data to tune $\delta$ and $\kappa_0$

!---

