# Multi-phase, multi-component radial injection

Theis pumping tests are described in the [page on pumping tests](porous_flow/tests/dirackernels/dirackernels_tests.md) and the [page on point sources](porous_flow/sinks.md).  This page describes similar tests, but in the multi-phase, multi-component setting.

## The similarity solution

Flow from a source in a radial 1D problem admits a similarity solution that is valid for a
broad range of problems, including multi-component, multi-phase flow. Properties such as fluid
pressure, saturation and component mass fraction can all be characterised by the similarity
variable $\zeta = r^2/t$, where $r$ is the radial distance from the source, and $t$ is time.

This similarity solution can be used to verify simple 1D radial models of increasing complexity.

## Two phase immiscible flow

The simplest test case that features a similarity solution is injection of a gas phase into a fully liquid
saturated model at a constant rate.  This is described [here](porous_flow/tests/dirackernels/dirackernels_tests.md).


## Two phase miscible flow

The similarity solution is also recovered even when including mutual dissolution of the gas and aqueous
phases. In this example, CO$_2$ is injected into a fully saturated reservoir at a constant rate, with
the phase conditions calculated using the WaterNCG fluid state material that implements the persistent
primary variable (in this case, the total mass fraction of CO$_2$ summed over all phases).  Initially, there
is insufficient CO$_2$ to form a gas phase, with all injected CO$_2$ dissolving into the resident water.

!listing modules/porous_flow/test/tests/fluidstate/theis_tabulated.i

After approximately 3000$\,$s, enough CO$_2$ has been injected to form a gas phase, and the problem
evolves from a single liquid phase to one including both gas and liquid phases.  [fig:theis_similarity_waterncg_fig] shows the comparison of similarity solutions calculated with
either fixed radial distance or fixed time. In this case, good agreement is observed between the two
results for both primary variables (gas pressure and total mass fraction of CO$_2$). The similarity
solution is also observed in [fig:theis_similarity_waterncg_fig2] for both the calculated
gas saturation and the mass fraction of CO$_2$ dissolved in the water.

Since this test is part of the automatic test suite, it needs to run rapidly.  Better agreement between the similarity solutions are obtained by refining the spatial and temporal resolution.

!media fluidstate/theis_similarity_waterncg_fig.png style=width:90%;margin-left:10px caption=Comparison of similarity solutions for the water-NCG fluid state. (a) Gas pressure; (b) Total mass fraction of CO2.  id=fig:theis_similarity_waterncg_fig

!media fluidstate/theis_similarity_waterncg_fig2.png style=width:90%;margin-left:10px caption=Comparison of similarity solutions for the water-NCG fluid state. (a) Gas saturation; (b) Dissolved CO2 mass fraction.  id=fig:theis_similarity_waterncg_fig2

## CO$_{2}$ injection into brine

Similar results are obtained for CO$_2$ injection in brine, using the BrineCO2 fluid state material.
In this example, CO$_2$ is injected into a reservoir at a constant rate. Initially, the reservoir is
fully saturated with brine which has a salt mass fraction of 0.1. During the early injection period,
there is insufficient CO$_2$ to form a gas phase, with all injected CO$_2$ dissolving into the resident
brine.

!listing modules/porous_flow/test/tests/fluidstate/theis_brineco2.i

After approximately 2000$\,$s, enough CO$_2$ has been injected to form a gas
phase, and the problem evolves from a single liquid phase to one including both
gas and liquid phases.  Note that this time is earlier than the problem with
water instead of brine, as the equilibrium saturation of CO$_2$ in  brine is
less than that in water. [fig:theis_similarity_brineco2_fig] shows
the comparison of similarity solutions calculated with either fixed radial
distance or fixed time. In this case, good agreement is observed between the two
results for both primary variables (gas pressure and total mass fraction of
CO$_2$). The similarity solution is also observed in [fig:theis_similarity_brineco2_fig2] for both the calculated gas saturation
and the mass fraction of CO$_2$ dissolved in the water.

This example is included in the automatic test suite, but is marked heavy due to the
run time.

!media fluidstate/theis_similarity_brineco2_fig.png style=width:50%;margin-left:10px caption=Comparison of similarity solutions for the brine-CO2 fluid state. (a) Gas pressure; (b) Total mass fraction of CO2; (c) NaCl mass fraction.  id=fig:theis_similarity_brineco2_fig

!media fluidstate/theis_similarity_brineco2_fig2.png style=width:50%;margin-left:10px caption=Comparison of similarity solutions for the brine-CO2 fluid state. (a) Gas saturation; (b) Dissolved CO2 mass fraction.  id=fig:theis_similarity_brineco2_fig2
