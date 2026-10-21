# Dead-oil waterflood

These examples show how to model the injection of water into a reservoir containing dead oil
(oil without dissolved gas). The oil properties are read from a table of the type used by
black-oil reservoir simulators, so they can be used as a starting point for models that use
existing PVT data.

## Fluid and rock properties

There are two phases and two components: water (phase 0, component 0) and dead oil
(phase 1, component 1). Neither component dissolves in the other phase, which is
described by the mass fractions

!listing modules/porous_flow/examples/dead_oil/waterflood_1d.i block=AuxVariables

The water is a [SimpleFluidProperties](SimpleFluidProperties.md) fluid with a viscosity of
0.5$\,$mPa$\,$s. The oil properties are given by
[TabulatedPVTFluidProperties](TabulatedPVTFluidProperties.md), using a PVDO-style table of
pressure, formation volume factor $B_{o}$ and viscosity $\mu_{o}$, together with the oil density at
standard conditions:

!listing modules/porous_flow/examples/dead_oil/waterflood_1d.i block=FluidProperties

The problem is formulated with the water porepressure and the oil saturation as the nonlinear
variables, using [PorousFlow2PhasePS](PorousFlow2PhasePS.md) with zero capillary pressure. The
porosity is 0.2, the permeability is $10^{-12}\,$m$^{2}$ (approximately 1 Darcy), and the relative
permeabilities are of [Corey](PorousFlowRelativePermeabilityCorey.md) type with exponent 2, a
connate water saturation of 0.2 and a residual oil saturation of 0.2:

!listing modules/porous_flow/examples/dead_oil/waterflood_1d.i block=Materials

The reservoir is initially at 20$\,$MPa, with oil at the connate water saturation, so only the oil
is mobile.

## One-dimensional waterflood: comparison with Buckley-Leverett

In the first example, water is injected at a constant mass rate of
$10^{-3}\,$kg$\,$m$^{-2}\,$s$^{-1}$ at the left end of a 100$\,$m long domain. Oil and water leave
through the right end, where each phase flows out in proportion to its mobility towards a fixed
pressure of 20$\,$MPa:

!listing modules/porous_flow/examples/dead_oil/waterflood_1d.i block=BCs

Without capillary pressure and gravity, and neglecting compressibility, the water saturation
satisfies the Buckley-Leverett equation
\begin{equation}
\phi \frac{\partial S_{w}}{\partial t} + u \frac{\partial f_{w}(S_{w})}{\partial x} = 0, \qquad
f_{w} = \frac{k_{r,w}/\mu_{w}}{k_{r,w}/\mu_{w} + k_{r,o}/\mu_{o}},
\end{equation}
where $u$ is the Darcy velocity of the injected water and $f_{w}$ is the water fractional flow.
Each saturation behind the front travels at speed $u f_{w}'(S_{w})/\phi$, and the saturation at
the front is found with the Welge tangent construction. The script `buckley_leverett.py` in the
example directory computes this solution and compares it with the PorousFlow results.
With the oil viscosity of 1.17$\,$mPa$\,$s at 20$\,$MPa, the saturation at the front is 0.528 and
the front reaches 25.2$\,$m after 25 days and 50.4$\,$m after 50 days.

!media porous_flow/dead_oil_waterflood_1d.png caption=Water saturation in the one-dimensional waterflood after 25 and 50 days: PorousFlow (points) and the Buckley-Leverett solution (lines). id=dead_oil_waterflood_1d_fig

The PorousFlow results agree with the Buckley-Leverett solution
([dead_oil_waterflood_1d_fig]). The front is smeared over a few elements by the
[numerical diffusion](numerical_diffusion.md) of the upwinded scheme, and becomes sharper as the
mesh is refined.

## Two-dimensional waterflood with wells

The second example is a 200$\,$m $\times$ 200$\,$m $\times$ 10$\,$m reservoir with an injection
well at $(20, 20)$ and a production well at $(180, 180)$, using the same fluid and rock properties.

Water is injected at a constant rate of 0.5$\,$kg$\,$s$^{-1}$ using a
[PorousFlowSquarePulsePointSource](PorousFlowSquarePulsePointSource.md). An injection
[PorousFlowPeacemanBorehole](PorousFlowPeacemanBorehole.md) is not used here, because the water
is immobile at the connate water saturation, so such a well would initially inject nothing.

The production well has a radius of 0.1$\,$m and a bottomhole pressure of 19.5$\,$MPa. It is
represented by one [PorousFlowPeacemanBorehole](PorousFlowPeacemanBorehole.md) for each phase, so
that each phase is produced in proportion to its mobility:

!listing modules/porous_flow/examples/dead_oil/waterflood_2d.i block=DiracKernels

!alert note
For `PorousFlowPeacemanBorehole`, `use_mobility = true` already includes the relative
permeability, so `use_relative_permeability` should not also be set. This differs from the
boundary condition [PorousFlowPiecewiseLinearSink](PorousFlowPiecewiseLinearSink.md) used in the
one-dimensional example, where the relative permeability is included separately with
`use_relperm`.

The oil and water production rates at surface conditions, the water cut and the cumulative oil
production are computed with postprocessors:

!listing modules/porous_flow/examples/dead_oil/waterflood_2d.i block=Postprocessors

!media porous_flow/dead_oil_waterflood_2d.png caption=Oil and water production rates at surface conditions (top) and water cut (bottom) for the two-dimensional waterflood. id=dead_oil_waterflood_2d_fig

The production history is shown in [dead_oil_waterflood_2d_fig]. After a brief initial period,
during which the pressure around the production well falls towards the bottomhole pressure, the
oil rate settles at about 41$\,$m$^{3}$/day at surface conditions. This corresponds to about
42.6$\,$m$^{3}$/day at reservoir conditions ($B_{o} = 1.038$), which balances the injected water
volume of about 42.8$\,$m$^{3}$/day. Water breaks through at the production well after about
470 days, when about 0.25 pore volumes have been injected. This is less than the 0.42 pore
volumes of the one-dimensional case, because the water reaches the production well before it has
swept the whole area. After breakthrough the oil rate falls and the water cut rises, reaching
about 0.56 after 2 years.

The script `plot_waterflood_2d.py` in the example directory plots the production history.
