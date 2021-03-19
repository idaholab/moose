# Hysteresis in PorousFlow

Hysteretic capillary pressure and relative permeability functions are implemented in PorousFlow.  Hysteresis means that capillary pressure and relative permeability depend on the *saturation history* as well as the current state.

The implementation of hysteresis in PorousFlow closely follows the TOUGH implementation as described in [!citet](doughty2007) and [!citet](doughty2008).  Some other pertinent material may be found in [!citet](jaynes1984), [!citet](niemi1988) and [!citet](finsterle1998).  In this implementation, capillary pressure at any point in the finite-element mesh is described by one of four "curves" (or functions), and relative permeability at any point is described by one of two "curves".  The capillary-pressure curves are illustrated in [histeretic_order_fig], which shall now be qualitatively described.

!media media/porous_flow/hysteretic_order.png caption=Illustration of a hysteretic capillary pressure function.  id=histeretic_order_fig

Assume that the point in question starts at full liquid saturation, $S_{l} = 1$.  Its history then proceeds as follows.

1. The liquid saturation is gradually reduced.  This is called "drying" or "draining".  The capillary pressure follows the *order zero* curve or *zeroth-order drying* curve, which is also termed the "primary drying" curve.
2. At some time, the liquid saturation begins to increase.  At this time, the liquid saturation $S_{l} = \mathrm{TP}_{0}$ in the figure ("TP" stands for "turning point", where $\partial S_{l}/\partial t = 0$).  Increasing liquid saturation is called "wetting" or "imbibing".  The capillary pressure follows the *order one* or *first-order wetting* curve.  This curve is an mixture of the primary drying curve and a modified version of the "primary wetting" curve.  The mathematical description of the mixture is described below.
3. At some later time, the liquid saturation begins to decrease again.  At this time, the liquid saturation $S_{l} = \mathrm{TP}_{1}$ in the figure.  The capillary pressure follows the *order two* or *second-order drying* curve.
4. At some later time, the liquid saturation beings to increase again.  The capillary pressure follows the *order three* or *third-order* curve.
5. At later times, the liquid saturation can decrease and increase, and provided the saturation obeys $\mathrm{TP}_{2} < S_{l} < \mathrm{TP}_{1}$, the capillary pressure follows the third-order curve.

The saturation history just described means that the order increases.  However, the order may also decrease, as illustrated in [histeretic_order_012_fig].

- The second-order wetting curve becomes the zeroth-order drying curve if $S_{l} \leq \mathrm{TP}_{0}$.
- The third-order curve becomes the second-order drying curve if $S_{l} \leq \mathrm{TP}_{2}$.
- The third-order curve becomes the first-order wetting curve if $S_{l} \geq \mathrm{TP}_{1}$.

!media media/porous_flow/hysteretic_order_012.png caption=Hysteretic capillary pressure curves.  This history is: (a) drying to $S_{l}$ $0.6$; (b) wetting to $S_{l}$ $0.9$; (c) drying to $S_{l}$ $0.3$, where the second-order drying curve is followed to $S_{l}$ $0.6$ and then the zeroth-order drying curve is followed; (d) wetting to $S_{l}$ $0.7$; (e) drying to $S_{l}$ $0.2$, where the second-order drying curve is followed to $S_{l}$ $0.3$ and then the zeroth-order drying curve is followed; (f) wetting to $S_{l}$ $0.4$; (g) drying to $S_{l}$ $0.1$ where the second-order drying curve is followed to $S_{l}$ $0.2$ and then the zeroth-order drying curve is followed.  id=histeretic_order_012_fig

The order is computed by the [PorousFlowHysteresisOrder](PorousFlowHysteresisOrder.md) Material, as discussed in detail below.

## Capillary curves

All capillary curves are based on the [van Genuchten](capillary_pressure.md) curve
\begin{equation}
\label{vg.cap.eqn}
P_{c}(S_{l}) = \frac{1}{\alpha} \left( S_{\mathrm{eff}}^{n/(1-n)} - 1\right)^{1/n} \ ,
\end{equation}
where
\begin{equation}
S_{\mathrm{eff}} = \frac{S_{l} - S_{l,min}}{1 - S_{gr}^{\Delta} - S_{l,min}} \ .
\end{equation}
In these formulae:

- $P_{c}$ is the capillary pressure, with SI units Pa.  It obeys $P_{c}\geq 0$.
- $S_{l}$ is the liquid saturation, which is dimensionless.
- $\alpha$ is a van Genuchten parameter, with SI units Pa$^{-1}$.  Two values may be entered by the user --- one describing the primary drying curve and the other for the primary wetting curve --- and both obey $\alpha > 0$.
- $n$ is a van Genuchten parameter, which is dimensionless.  Two values may be entered by the user --- one describing the primary drying curve and the other for the primary wetting curve -- and both obey $n>1$.
- $S_{l,min}$ is the minimum saturation for which the van Genuchten expression is valid.  It is a dimensionless user-input and obeys $0\leq S_{l,min} < 1$.  Because $n/(1-n) < 0$, $P_{c}$ behaves like $P_{c}\rightarrow\infty$ as $S_{l} \rightarrow S_{l,min}^{+}$.  It is sometimes numerically advantageous and/or physically necessary to define extensions of $P_{c}$ for $S_{l}<S_{l,min}$, which is discussed below.  Two extensions are shown in [histeretic_cap_extensions_fig]
- $S_{gr}^{\Delta}$ is the residual gas saturation, and $1-S_{gr}^{\Delta}$ is the maximum saturation for which the van Genuchten expression is valid.  It is dimensionless but it is not a user input (see more details below).  It obeys $S_{gr}^{\Delta} \geq 0$ and $1-S_{gr}^{\Delta} > S_{l,min}$.  As $S_{l}\rightarrow 1-S_{gr}^{\Delta}-$, $P_{c} \rightarrow 0$.  It is sometimes numerically advantageous and/or physically necessary to define extensions of $P_{c}$ for $S_{l}> 1-S_{gr}^{\Delta}$ as described below, as shown in [histeretic_cap_extensions_fig].

The primary drying curve uses
\begin{equation}
S_{gr}^{\Delta} = 0 \ .
\end{equation}
The primary wetting curve uses
\begin{equation}
S_{gr}^{\Delta} = S_{gr}^{max} \ ,
\end{equation}
where $0 \leq S_{gr}^{max} < 1 - S_{l,min}$ is a user input (as noted below $S_{gr}^{max}$ must be stricly positive for the cubic-spline modification of the water wetting relative permeability to be well-defined).  The other curves use other values of $S_{gr}^{\Delta}$ as described below.

!media media/porous_flow/hysteretic_cap_extensions.png caption=Various extensions of the hysteretic capillary pressure curve available in PorousFlow.  The primary wetting curve is shown, which has the property that $P_{c}\rightarrow 0$ as $S\rightarrow 1- S_{gr}^{max}$ (recall that the primary wetting curve uses the user-inputted $S_{gr}^{max}$ for $S_{gr}^{\Delta}$).  id=histeretic_cap_extensions_fig

### Lower extensions

Two types of lower (small $S_{l}$) extensions are available [!citep](doughty2008).  They are based on the user inputting the value
\begin{equation}
P_{c}^{\mathrm{max}} > 0 \ ,
\end{equation}
which is the value at which extension commences (that is, all $P_{c} > P_{c}^{\mathrm{max}}$ will use the extension, not the original van Genuchten expression [vg.cap.eqn]).  [histeretic_cap_extensions_fig] shows examples for $P_{c}^{\mathrm{max}} = 1.1$.  Both extensions are designed so that the resulting curve is continuous and its derivative is continuous.

- Quadratic: $P_{c}$ is a quadratic in $S_{l}$ that satisfies $\mathrm{d}P_{c}/\mathrm{d}S_{l} = 0$ at $S_{l} = 0$.
- Exponential: $P_{c}$ is an exponential in $S_{l}$.

Lower extensions could be necessary because the model could be heated to evaporate the water, or chemical reactions could occur to reduce $S_{l}$ below $S_{l,min}$.  They could also be necessary because the region $S \leq S_{l,min}$ might be explored as the MOOSE solver converges towards a solution.

### Upper extensions

One type of upper extension (large $S_{l}$) is available [!citep](doughty2008).  It is based on the user inputting a value
\begin{equation}
0 < \mathrm{UpperRatio} < 1 \ .
\end{equation}
When $S_{l} > \mathrm{UpperRatio} \times (1 - S_{gr}^{\Delta})$ then the upper extension is used:
\begin{equation}
P_{c} \propto \left (\frac{1 - S_{l}}{1 - \mathrm{UpperRatio}\times ( 1- S_{gr}^{\Delta})} \right)^{\beta} \ ,
\end{equation}
where the coefficient of proportionality and $\beta$ are chosen to ensure the result is C1-continuous.  [histeretic_cap_extensions_fig] shows an example for $\mathrm{UpperRatio} = 0.9$.  This type of extension has the property that $P_{c}(1) = 0$.  This type of extension is not necessary for the primary drying curve.

Upper extensions could be necessary to help the MOOSE solver converge towards a solution.

### Zeroth-order drying curve

This is defined by [vg.cap.eqn] (along with an optional lower extension) with $S_{gr}^{\Delta} = 0$.

### First-order wetting curve

The idea here is to use a wetting curve (with nonzero $S_{gr}^{\Delta}$ and the wetting $\alpha$ and $n$ parameters) but with a value of liquid saturation that ensures continuity.

The first-order wetting curve uses a non-zero value for the quantity $S_{gr}^{\Delta}$ in [vg.cap.eqn].
This is [!citep](doughty2007)
\begin{equation}
\label{eqn.land.1}
S_{gr}^{\Delta} = \frac{1 - \mathrm{TP}_{0}}{1 + (\frac{1}{S_{gr}^{max}} - \frac{1}{1 - S_{l,r}})(1 - \mathrm{TP}_{0})} \ .
\end{equation}
This is the so-called "Land" expression.  It depends on $S_{l,r}$, which obeys $0 \leq S_{l,r} < 1$ and is the "liquid residual saturation" defined to be the saturation at which the liquid relative permeability tends to zero.  The following may be noticed:

- if $S_{l} = 1 - S_{gr}^{\Delta}$ then $P_{c} = 0$ (assuming no upper extension).  So $S_{l} = 1 - S_{gr}^{\Delta}$ is the saturation point at which the curves on [histeretic_order_fig] tend to zero.
- if $\mathrm{TP}_{0}$ is close to 0, then $S_{gr}^{\Delta}$ is close to 1.  This means that the first-order wetting curve will be quite close to the zeroth-order drying curve.
- if $\mathrm{TP}_{0}$ is close to $S_{l,min}$ then $S_{gr}^{\Delta}$ is close to $S_{gr}^{max}$.  This means that the first-order wetting curve will be quite close to the primary wetting curve.

These points are illustrated in [histeretic_different_tps_fig].

!media media/porous_flow/hysteretic_different_tps.png caption=Various first-order curves resulting from different turning points.  Notice the different values of $S_{gr}^{\Delta}$ which depend on the turning-point saturation.  id=histeretic_different_tps_fig

However, if [vg.cap.eqn] is used with $S_{gr}^{\Delta}$, the result will be discontinuous at the turning point.  This may be seen in [histeretic_order1_fig]: the result would jump from the red drying curve to the blue wetting curve.

!media media/porous_flow/hysteretic_order1.png caption=Construction of the first-order curve.  id=histeretic_order1_fig

To ensure continuity, define (see [histeretic_order1_fig]):

- $P_{c}^{\Delta}$, which is the value of $P_{c}$ at $\mathrm{TP}_{0}$, using the primary drying curve.
- $S_{l,wet}^{\Delta}$, which is the value of $S_{l}$ at the capillary pressure $P_{c}^{\Delta}$ defined using [vg.cap.eqn] using the wetting $\alpha$ and $n$ parameters and $S_{gr}^{\Delta}$

Armed with these quantities, the first-order wetting curve is defined by [vg.cap.eqn] using the following:

- the $\alpha$ and $n$ parameters for the primary wetting curve
- $S_{gr}^{\Delta}$ defined by the Land expression [eqn.land.1]
- $\tilde{S}_{l}$ instead of $S_{l}$, to ensure continuity of the result.  Here

\begin{equation}
\label{eqn.1st.order.sl}
\tilde{S}_{l} = S_{l,wet}^{\Delta} + (S_{l} - \mathrm{TP}_{0}) \frac{1 - S_{gr}^{\Delta} - S_{l,wet}^{\Delta}}{1 - S_{gr}^{\Delta} - \mathrm{TP}_{0}} \ .
\end{equation}
This is designed so that

- when $S_{l} = \mathrm{TP}_{0}$ then $\tilde{S}_{l} = S_{l,wet}^{\Delta}$ and
- when $S_{l} = 1 - S_{gr}^{\Delta}$ then $\tilde{S}_{l} = 1 - S_{gr}^{\Delta}$.

The result is the green curve in [histeretic_order1_fig] which smoothly transitions from the red drying curve and the blue wetting curve.


### Second-order drying

This is drawn from Eqn(3) of [!citet](niemi1988) and Eqn(5) of [!citet](finsterle1998) but may be slightly different because their explanations are a little opaque.  The idea is similar to the first-order case: use the drying curve, but with a different saturation, $\tilde{S}_{l}$, to ensure continuity of the result.

Define

- $S_{d}$ to be the saturation on the primary drying curve corresponding to $P_{c}$ at $\mathrm{TP}_{1}$
- $\tilde{S}_{l}$ to smoothly interpolate between $\mathrm{TP}_{0}$ when $S_{l} = \mathrm{TP}_{0}$, and $S_{d}$ when $S_{l} = \mathrm{TP}_{1}$, ie

\begin{equation}
\label{eqn.2nd.order.sl}
\tilde{S}_{l} = \mathrm{TP}_{0} + (S_{l} - \mathrm{TP}_{0}) \frac{S_{d} - \mathrm{TP}_{0}}{\mathrm{TP}_{1} - \mathrm{TP}_{0}} \ .
\end{equation}

The second-order drying curve uses:

- the $\alpha$ and $n$ parameters of the primary drying curve
- $S_{gr}^{\Delta} = 0$
- $\tilde{S}_{l}$ instead of $S_{l}$, as defined by [eqn.2nd.order.sl]

### Third-order curves

The approach here is to take an appropriate average of the first-order wetting curve and the second-order drying curve, which is probably the same as [!citet](niemi1988), [!citet](finsterle1998) and [!citet](doughty2007), although details are sparse in those references.

Define

- $P_{c}^{w}$ is the first-order wetting capillary pressure using $\tilde{S}_{l}$ defined by [eqn.1st.order.sl]
- $P_{c}^{d}$ is the second-order drying capillary pressure using $\tilde{S}_{l}$ defined by [eqn.2nd.order.sl]

Then, the third-order capillary pressure is defined by
\begin{equation}
\log P_{c} = \log P_{c}^{w} + (s - \mathrm{TP}_{1}) \frac{\log P_{c}^{d} - \log P_{c}^{w}}{\mathrm{TP}_{0} - \mathrm{TP}_{1}} \ .
\end{equation}

This produces continuous capillary-pressure functions as shown in [histeretic_order_fig].


## Relative permeabilities

Hysteresis is defined for one-phase and two-phase systems only.  The phase is assumed to be the liquid phase in one-phase models.  The liquid and gas relative permeability functions are both hysteretic.  Only the drying and first-order wetting curves are defined.  This means that if the system dries (following the drying curve) and then wets (following the first-order wetting curve) and then subsequently dries, the system will move along the first-order wetting curve until the turning point is reached, when it will move along the drying curve.

The starting point for the relative permeability functions for liquid ($k_{r,l}$) and gas ($k_{r,g}$) are of the [van Genuchten](relative_permeability.md) form [!citet](doughty2007)
\begin{equation}
\begin{aligned}
\label{relperm.eqns}
k_{r,l}(S_{l}) & = \sqrt{\bar{S}_{l}} \left[1 - \left(1 - \frac{\bar{S}_{gt}}{1 - \bar{S}_{l}^{\Delta}}\right) \left( 1- (\bar{S}_{l} + \bar{S}_{gt})^{1/m} \right)^{m} - \frac{\bar{S}_{gt}}{1 - \bar{S}_{l}^{\Delta}} \left(1 - (\bar{S}_{l}^{\Delta})^{1/m} \right)^{m} \right]^{2} \ , \\
k_{r,g}(S_{l}) & = k_{r,g}^{max} \left(1 - \bar{S}_{l} - \bar{S}_{gt}\right)^{\gamma} \left(1 - (\bar{S}_{l} + \bar{S}_{gt})^{1/m} \right)^{2m} \ .
\end{aligned}
\end{equation}
In these expressions
\begin{equation}
\begin{aligned}
\bar{S}_{l} & = \frac{S_{l} - S_{l, r}}{1 - S_{l, r}} \ , \\
\bar{S}_{l}^{\Delta} & = \frac{\mathrm{TP}_{0} - S_{l, r}}{1 - S_{l, r}} \ , \\
\bar{S}_{gt} & = \frac{S_{gr}^{\Delta} (S_{l} - \mathrm{TP}_{0})}{(1 - S_{l, r})(1 - \mathrm{TP}_{0} + S_{gr}^{\Delta})} \ .
\end{aligned}
\end{equation}
Here

- $S_{l, r}$ is the residual liquid saturation, which marks the point at which the liquid relative permeability becomes zero.  It is a dimensionless user-input, which satisfies $S_{l,min} \leq S_{l, r} < 1$.
- $k_{r,g}^{max}$ is the maximum gas relative permeability, which occurs on the drying curve for $S_{l} \leq S_{l, r}$.  It is a dimensionless user-input, which satisfies $0 < k_{r, g}^{max} \leq 1$.  Most frequently $k_{r, g}^{max} = 1$ is chosen.
- $\gamma$ is a dimensionless index, inputted by the user, which satisfies $\gamma > 0$.  Most frequently $\gamma \approx 1/3$ is chosen.
- $m$ is a dimensionless index, inputted by the user, which satisfies $m>0$.

### Extending and modifying the functions

An immediate problem with the functions given in [relperm.eqns] is that they are not defined for $S_{l} < S_{l, r}$ (meaning that $\bar{S}_{l} < 0$).  The wetting curve for water also has an infinite derivative.  These behaviors are obviously undesirable in a numerical implementation, and are probably unphysical, so the functions need to be extended and modified.

For reference, the zeroth-order (drying) are given by [relperm.eqns] but because $S_{gr}^{\Delta} = 0$ in this case, $\bar{S}_{gt} = 0$, and the functions reduce to
\begin{equation}
\begin{aligned}
\label{relperm.eqns.0}
k_{r,l}(S_{l}) & = \sqrt{\bar{S}_{l}} \left[1 - \left( 1- \bar{S}_{l}^{1/m} \right)^{m}\right]^{2} \ , \\
k_{r,g}(S_{l}) & = k_{r,g}^{max} \left(1 - \bar{S}_{l}\right)^{\gamma} \left(1 - \bar{S}_{l}^{1/m} \right)^{2m} \ .
\end{aligned}
\end{equation}

Assuming the relative permeability values are constant outside the well-defined domain results in curves of the form shown in [histeretic_krel_unextended_fig].

!media media/porous_flow/hysteretic_krel_unextended.png caption=The basic, unextended relative permeability curves.  id=histeretic_krel_unextended_fig

[!citet](doughty2008) recommends extending and modifying the curves in the following ways.

1. There is no hysteresis in the region $S_{l} < S_{l, r}$: the drying curves equal the wetting curves in this region, and if $\mathrm{TP}_{0} < S_{l, r}$ then $\mathrm{TP}_{0} = S_{l, r}$ is used in the wetting curves. 

2. The gas relative permeability is extended to the region $S_{l} < S_{l, r}$ so that $k_{r, g}(0) = 1$.   Two types of extension are possible in MOOSE.  Both are cubic functions satisfying $k_{r, g}(0) = 1$, $k_{r, g}(S_{l, r}) = k_{r, g}^{max}$ and $k'_{r, g}(0) = 0$:

   - A "linear-like" extension, where the cubic's derivative at $S_{l} = S_{l, r}$ equals $(k_{r, g}^{max} - 1)/S_{l, r}$.  That is, the derivative is equal to the average slope in the extended region.  This means the final result has a discontinuous derivative at $S_{l, r}$, but the result often "looks better" to the eye (see figures below).
   - A "cubic" extension, where the cubic's derivative at $S_{l} = S_{l, r}$ equals the primary drying-curve's derivative at that point.  This means the drying curve is C1 continuous (the wetting curve is not).

3. The water wetting curve is modified around the point $1 - S_{gr}^{\Delta}$, which is the point of infinite derivative.  Firstly, it is assumed that $S_{gr}^{max} > 0$.  If this is not the case then the following modification will not work, but may be unecessary anyway.  Two points are defined

   - $S_{\mathrm{small}} = r(1 - S_{gr}^{\Delta})$, where $r$ is a (dimensionless) user-input satisfying $0 < r < 1$, and optimally $r$ should be close to 1, for example $r=0.9$.
   - $S_{\mathrm{big}} = 1 - S_{gr}^{\Delta} / 2$.  Note that [!citet](doughty2008) defines $S_{\mathrm{big}} = (2 - r)(1 - S_{gr}^{\Delta})$, but this can be greater than 1 and can result in a poor modification.

 Then:

   - for $S<S_{\mathrm{small}}$ the liquid wetting curve, given in [relperm.eqns], is used
   - for $S>S_{\mathrm{big}}$ the liquid drying curve, given in [relperm.eqns.0], is used
   - otherwise, a cubic spline interpolates between these two, with parameters chosen so that the final result is continuous and has a continuous derivative.

The result is illustrated in [histeretic_krel_extended_fig], using a linear-like extension for the gas relative permeability curve, and $r = 0.9$.

!media media/porous_flow/hysteretic_krel_extended.png caption=Extended and modified relative permeability curves, using a linear-like extension for the gas relative permeability curve, and $r$ being 0.9.  id=histeretic_krel_extended_fig

### Examples

!media media/porous_flow/hysteretic_krel_example_1.png caption=Example hysteretic relative permeability functions.  The system initialises at full saturation, dries to saturation 0.5 (greater than $S_{l, r}$) and then wets back to saturation 1.  id=histeretic_krel_example_1

!media media/porous_flow/hysteretic_krel_example_2.png caption=Example hysteretic relative permeability functions.  The system initialises at full saturation, dries to saturation 0.1 (less than $S_{l, r}$) and then wets back to saturation 1.  In the extended region ($S_{l} \leq S_{l, r}$), the drying curve equals the wetting curve.  id=histeretic_krel_example_2

## Exploring hysteresis using the python script

A python script that produces plots of hysteretic capillary pressure and relative permeability has been included in the MOOSE repository.  This allows users to quickly explore the consequences of choosing different extension strategies to the capillary and relative-permeability curves, as well as the impact of hysteresis order on the shape of the curves.  It was used to produce all the figures displayed on this page.  It is

!listing modules/porous_flow/test/tests/hysteresis/hys.py


## Input file syntax for hysteretic capillarity

To include hysteretic capillarity in an existing (non-hysteretic) input file, the following changes need to be made.

1. Any capillary-pressure UserObjects, such as [PorousFlowCapillaryPressureVG](PorousFlowCapillaryPressureVG.md) will no-longer be used, so should be removed from the input file for clarity.
2. A [PorousFlowHysteresisOrder](PorousFlowHysteresisOrder.md) Material needs to be included.
3. A Material that computes the porepressure(s) and saturation(s) needs to be included.

   - For 1-phase partially-saturated situations, [PorousFlow1PhaseP](PorousFlow1PhaseP.md) should be removed and replaced by [PorousFlow1PhaseHysP](PorousFlow1PhaseHysP.md).  Note the van Genuchten parameter input is $m$ for the non-hysteretic version, but $n$ for the hysteretic version, where $n = 1/(1 - m)$ and $m = 1 - 1/n$.
   - For 2-phase situations using the two porepressures as the primary variables, [PorousFlow2PhasePP](PorousFlow2PhasePP.md) should be removed and replaced by [PorousFlow2PhaseHysPP](PorousFlow2PhaseHysPP.md).
   - For 2-phase situations using the liquid porepressure and gas saturation as the primary variables, [PorousFlow2PhasePS](PorousFlow2PhasePS.md) should be removed and replaced by [PorousFlow2PhaseHysPS](PorousFlow2PhaseHysPS.md).

An example is:

!listing modules/porous_flow/test/tests/hysteresis/1phase_3rd.i start=[hys_order_material] end=[Postprocessors]

### Preliminary example

Before introducing hysteresis into an input file, it might be useful to assess how hysteresis evolves during model evolution using a [PorousFlowHystereticInfo](PorousFlowHystereticInfo.md) Material.  This Material *does not* compute porepressures or saturations, but instead allows users to visualise capillary pressure.  Hence, the following input file *does not* perform a usual PorousFlow simulation, but instead allows a preliminary exploration of hysteresis:

!listing modules/porous_flow/test/tests/hysteresis/vary_sat_1.i

By changing the `FunctionAux` that controls the saturation, various hysteretic curves may be constructed.

!listing modules/porous_flow/test/tests/hysteresis/vary_sat_1.i start=[sat_aux] end=[hys_order]

[hys_vary_sat_1_fig] shows the results of two hysteretic simulations.  Both are initialised at full saturation and are then dried.  The first (green curve) dries to $S_{l} = 0$ before wetting, so follows the primary wetting curve.  The second (yellow curve) dries to $S_{l} = 0.2$ before wetting, so follows a first-order wetting curve.

!media media/porous_flow/hys_vary_sat_1.png caption=The results of two hysteretic simulations.  The lines show the expected result (from the python script) while the crosses and asterisks show the MOOSE result.  id=hys_vary_sat_1_fig

Careful examination of [hys_vary_sat_1_fig] will reveal a subtle feature of the implementation of hysteresis in PorousFlow.  As the system dries and then re-wets, it follows the primary drying curve for the first time-step after the turning point.  This is why there is no asterisk at $(S_{l}, P_{c}) \approx (0.3, 1)$: instead it appears on the primary drying curve at $(S_{l}, P_{c}) \approx (0.3, 2)$.  Similarly, there is no cross around $(0.1, 1)$.  Consequences of this are discussed in a section below.

[hys_vary_sat_1_3rdorder_fig] results when the FunctionAux is

```
if(t <= 0.4, 1 - 2 * t, if(t <= 0.7, 2 * t - 0.6, if(t <= 0.95, 0.8 - 2 * (t - 0.7), 0.3 + 2 * (t - 0.95))))
```

!media media/porous_flow/hys_vary_sat_1_3rdorder.png caption=The result of a hysteretic simulation where the system is dried, then re-wet, then dried, then re-wet, so that it follows the zeroth, first, second and third-order curves  id=hys_vary_sat_1_3rdorder_fig

### Single-phase example

A simulation that simply removes and adds water to a system to observe the hysteretic capillary pressure is explored in this section.  The water flux is controlled by the Postprocessor

!listing modules/porous_flow/test/tests/hysteresis/1phase_3rd.i start=[flux] end=[hys_order]

and the DiracKernel

!listing modules/porous_flow/test/tests/hysteresis/1phase_3rd.i block=DiracKernels

The remainder of the input file is standard, with the inclusion of the hysteretic capillary pressure:

!listing modules/porous_flow/test/tests/hysteresis/1phase_3rd.i start=[hys_order_material] end=[Postprocessors]

The result is [hys_1phase_3_fig].

!media media/porous_flow/hys_1phase_3.png caption=The result of a single-phase simulation in which an external pump removes and adds water to a porous material in order to observe the hysteretic capillary pressure.  id=hys_1phase_3_fig

### Two-phase example using the PP formulation

A simulation that simply adds gas then removes gas from a 2-phase system to observe the hysteretic capillary pressure is explored in this section.  The gas flux is controlled by the Postprocessor

!listing modules/porous_flow/test/tests/hysteresis/2phasePP.i start=[flux] end=[hys_order]

and the DiracKernel

!listing modules/porous_flow/test/tests/hysteresis/2phasePP.i block=DiracKernels

The remainder of the input file is standard, with the inclusion of the hysteretic capillary pressure:

!listing modules/porous_flow/test/tests/hysteresis/2phasePP.i start=[hys_order_material] end=[Postprocessors]

The result is [hys_2phasePP_1_fig].

!media media/porous_flow/hys_2phasePP_1.png caption=The result of a two-phase simulation using a PP formulation in which an external pump adds to and removes gas from a porous material in order to observe the hysteretic capillary pressure.  id=hys_2phasePP_1_fig

### Two-phase example using the PS formulation

A simulation that simply adds gas, then removes gas, and adds it again to a 2-phase system to observe the hysteretic capillary pressure is explored in this section.  The gas flux is controlled by the Postprocessor

!listing modules/porous_flow/test/tests/hysteresis/2phasePS_2.i start=[flux] end=[hys_order]

and the DiracKernel

!listing modules/porous_flow/test/tests/hysteresis/2phasePS_2.i block=DiracKernels

The remainder of the input file is standard, with the inclusion of the hysteretic capillary pressure:

!listing modules/porous_flow/test/tests/hysteresis/2phasePS_2.i start=[hys_order_material] end=[Postprocessors]

The result is [hys_2phasePS_2_fig].

!media media/porous_flow/hys_2phasePS_2.png caption=The result of a two-phase simulation using a PS formulation in which an external pump adds, removes and then adds gas to a porous material in order to observe the hysteretic capillary pressure.  id=hys_2phasePS_2_fig


## Hysteretic capillary-pressure implementation remarks

As mentioned above, [hys_vary_sat_1_fig] reveals a subtle feature of the implementation of hysteresis in PorousFlow.  There is no asterisk around $(S_{l}, P_{c}) = (0.3, 1)$.  Instead it appears on the primary drying curve at around $(S_{l}, P_{c}) = (0.3, 2)$.  As the saturation is reduced along the drying curve, and then increased again, PorousFlow follows the primary drying curve for the first time-step after the turning point.  At subsequent steps, it follows the correct curve.

This is because the computation of hysteresis order *lags one timestep behind* the MOOSE simulation.  This is to ensure reasonable convergence behavior, as mentioned in [!citet](doughty2007) and [!citet](doughty2008).

The lagging behaviour has the unfortunate side-effect that simulations involving hysteretic capillary pressure do not conserve fluid mass.  The remainder of PorousFlow conserves fluid mass exactly, at all times.  However, when dealing with hysteretic capillary pressures, a small amount of mass is lost or gained whenever a turning point is encountered.  This should have limited impact upon models if saturations do not change excessively during a single time-step.

## Input file syntax for hysteretic relative permeability

To include hysteretic relative permeability in an existing (non-hysteretic) input file, the following changes need to be made

1. A [PorousFlowHysteresisOrder](PorousFlowHysteresisOrder.md) Material needs to be included.
2. Materials are needed to compute the relative permeability.  The existing Materials, such as [PorousFlowRelativePermeabilityVG](PorousFlowRelativePermeabilityVG.md) need to be replaced with [PorousFlowHystereticRelativePermeabilityLiquid](PorousFlowHystereticRelativePermeabilityLiquid.md), and, for 2-phase simulations [PorousFlowHystereticRelativePermeabilityGas](PorousFlowHystereticRelativePermeabilityGas.md).

A 1-phase example is:

!listing modules/porous_flow/test/tests/hysteresis/1phase_relperm.i start=[hys_order_material] end=[Postprocessors]

!alert note
Models need not contain both hysteretic capillary pressures and hysteretic relative permeabilities: the hysteresis may only appear in the capillary pressure, or the relative permeabilities.

### Single-phase examples

A simulation that simply removes and adds water to a system to observe the hysteretic relative permeability is explored in this section.  The water flux is controlled by the Postprocessor

!listing modules/porous_flow/test/tests/hysteresis/1phase_relperm.i start=[flux] end=[hys_order]

and the DiracKernel

!listing modules/porous_flow/test/tests/hysteresis/1phase_relperm.i block=DiracKernels

The remainder of the input file is standard, with the inclusion of the hysteretic capillary pressure:

!listing modules/porous_flow/test/tests/hysteresis/1phase_relperm.i start=[hys_order_material] end=[Postprocessors]

The result is [hys_1phase_relperm_fig].  By altering the `flux`, the system may be dried, re-wet, dried and re-wet again, to generate results stuch as [hys_1phase_relperm_2_fig]

!media media/porous_flow/hys_1phase_relperm.png caption=The result of a single-phase simulation in which an external pump removes and adds water to a porous material in order to observe the hysteretic relative permeability.  id=hys_1phase_relperm_fig

!media media/porous_flow/hys_1phase_2_relperm.png caption=The result of a single-phase simulation in which an external pump removes water until $S_{l} \approx 0.7$, then adds water until $S_{l} \approx 0.8$, then removes water until $S_{l} \approx 0.02$, then adds water until full saturation is reached.  id=hys_1phase_relperm_2_fig


### Two-phase examples

A simulation that simply adds and removes gas from a system to observe the hysteretic relative permeability is explored in this section.  The water flux is controlled by the Postprocessor

!listing modules/porous_flow/test/tests/hysteresis/2phasePS_relperm.i start=[flux] end=[hys_order]

and the DiracKernel

!listing modules/porous_flow/test/tests/hysteresis/2phasePS_relperm.i block=DiracKernels

The remainder of the input file is standard, with the inclusion of the hysteretic relative permeabilities:

!listing modules/porous_flow/test/tests/hysteresis/2phasePS_relperm.i start=[hys_order_material] end=[Postprocessors]

The result is [hys_2phasePS_relperm_fig].  By altering the `flux`, `k_rg_max` and `gas_low_extension_type`, the impact of various extensions may be explored, as shown in [hys_2phasePS_relperm_2_cubic_fig], [hys_2phasePS_relperm_2_linear_like_fig] and [hys_2phasePS_relperm_2_none_fig].

!media media/porous_flow/hys_2phasePS_relperm.png caption=The result of a two-phase simulation in which an external pump adds and removes gas from a porous material in order to observe the hysteretic relative permeability.  id=hys_2phasePS_relperm_fig

!media media/porous_flow/hys_2phasePS_relperm_2_cubic.png caption=The result of a two-phase simulation in which an external pump adds and removes gas from a porous material in order to observe the hysteretic relative permeability.  A cubic extension is used for the gas relative permeability.  id=hys_2phasePS_relperm_2_cubic_fig

!media media/porous_flow/hys_2phasePS_relperm_2_linear_like.png caption=The result of a two-phase simulation in which an external pump adds and removes gas from a porous material in order to observe the hysteretic relative permeability.  A linear-like extension is used for the gas relative permeability.  id=hys_2phasePS_relperm_2_linear_like_fig

!media media/porous_flow/hys_2phasePS_relperm_2_none.png caption=The result of a two-phase simulation in which an external pump adds and removes gas from a porous material in order to observe the hysteretic relative permeability.  No extension is used for the gas relative permeability, since $k_{r,g}^{max}$ is unity.  id=hys_2phasePS_relperm_2_none_fig



