# Point and line sources/sinks

A number of sources/sinks are available in Porous Flow, implemented as `DiracKernels`.  This page may be read in conjunction with [the description of some Dirac Kernel tests](porous_flow/tests/dirackernels/dirackernels_tests.md).  Descriptions of tests of point sources and sinks in multi-phase, multi-component scenarios may be found [here](porous_flow/tests/fluidstate/fluidstate_tests.md).

## Constant point source

[`PorousFlowSquarePulsePointSource`](/PorousFlowSquarePulsePointSource.md)
implements a constant mass point source that adds (removes) fluid at a constant
mass flux rate (kg.s$^{-1}$) for times between the specified start and end times. If
no start and end times are specified, the source (sink) starts at the
start of the simulation and continues to act indefinitely.
For instance:

!listing modules/porous_flow/test/tests/dirackernels/squarepulse1.i block=DiracKernels

Note that the parameter ```mass_flux``` is positive for a source and negative
for a sink, which is dissimilar to the conventions used below for line-sink strength $s$ and flux $f$.

## Point source from postprocessor

A mass point source can be specified via a value computed by a postprocessor using [`PorousFlowPointSourceFromPostprocessor`](/PorousFlowPointSourceFromPostprocessor.md) Dirac kernel.
Users have to make sure that the postprocessor is evaluated at ```timestep_begin``` so that the correct value is used within the timestep.

Such example is shown here:

!listing modules/porous_flow/test/tests/dirackernels/frompps.i block=DiracKernels

!listing modules/porous_flow/test/tests/dirackernels/frompps.i block=Postprocessors

Note that the parameter ```mass_flux``` is positive for a source and negative for a sink.


### Injecting fluid at specified temperature

When injecting a fluid at specified temperature (also computed as a postprocessor), users can add another Dirac
kernel [`PorousFlowPointEnthalpySourceFromPostprocessor`](/PorousFlowPointEnthalpySourceFromPostprocessor.md).
(Alternately, users can also fix the temperature of an injected fluid using a Dirichlet BC, but this adds/subtracts
heat energy to entire nodal volumes of porous material and fluid, so may lead to unacceptable errors
from the additional heat energies added/removed.)

Such example is shown here:

!listing modules/porous_flow/test/tests/dirackernels/hfrompps.i block=DiracKernels

!listing modules/porous_flow/test/tests/dirackernels/hfrompps.i block=Postprocessors


## PorousFlow polyline sinks in general

Two types of polyline sinks are implemented in PorousFlow: the
[`PorousFlowPolyLineSink`](/PorousFlowPolyLineSink.md) and the
[`PorousFlowPeacemanBorehole`](/PorousFlowPeacemanBorehole.md).  These
are extensions and specialisations of the general PorousFlowLineSink (that is not available
to use in an input file) which is described in this section.

Polyline sinks and sources are modelled as sequences of discrete points:
\begin{equation}
\textrm{polyline}\sim \left\{x_{0},\ x_{1},\ x_{2},\ldots,x_{N}\right\} \ .
\end{equation}
The sink is
\begin{equation}
\label{eq:line_sink}
s = \sum_{i}f(P_{i}, T_{i})w_{i}\delta(x - x_{i}) \ .
\end{equation}
Here $s$ is a volume source, measured in kg.m$^{-3}$.s$^{-1}$ (or J.m$^{-3}$.s$^{-1}$ for heat flow),
which when integrated over the finite element yields just the "sink strength", $f$, which has units
kg.s$^{-1}$ for fluid flow, or J.s$^{-1}$ for heat flow.  The sink strength $f$ is the parameter that is specified by the user in the input file, and the $w$ parameters are convenient weights that are discussed below.

The strength, $f$, is a function of porepressure and/or temperature, and may involve other quantities, as
enumerated below. The convention followed is:

- A sink has $s>0$.  This removes fluid or heat from the simulation domain;
- A source has $s<0$.  This adds fluid or heat to the simulation domain.

There are two separate input formats for the PorousFlow polyline sink. The first requires the location for each point along the line length to be specified. The second is relevant for straight lines only, and requires a starting location, direction and length.

The first input format can be defined by either a plain text file or a reporter. In the plain text file, each point is specified by a line containing the following space-separated quantities:
\begin{equation}
\label{eq:bh_plaintext_format}
{\mathtt{w_{i}\ x_{i}\ y_{i}\ z_{i}}}
\end{equation}

The weighting terms, $w_{i}$, are for user convenience, but for the Peaceman borehole case they are
the borehole radius at point $x_{i}$.

The reporter format for point data supplies the same coordinate and weighting data using the following syntax:

!listing modules/porous_flow/test/tests/dirackernels/pls02reporter.i start=weight_reporter end=piecewise-linear

where the polyline coordinates and weighting are defined in the following [ConstantReporter](/ConstantReporter.md):

!listing modules/porous_flow/test/tests/dirackernels/pls02reporter.i block=Reporters

Reporter input provides an easy way to control polyline sink point locations from a [Sampler](Samplers/index.md) multi-app.  
It is an error to supply both plaint text file and reporter input for the point data.

Rather than manually specifying each point via the separate points file or reporter, the second input format allows the
line to be specified using the combination of the following parameters:

- `line_base = '[w] [x] [y] [z]'`: the base/start point for the line
- `line_direction = '[dx] [dy] [dz]'`: line direction (does not need to be unit-length)
- `line_length = [length]`: exactly what you expect - the line length.

It is an error to specify both a point (plain text file or reporter) parameter, and the
line base parameter.  When specifying the line this way, one point will be
generated along the line for each element the line passes through.  These
points are automatically updated when the mesh changes due to adaptivity,
displacement, etc.  When using this mode of line-specification, the line end
points must NOT ever lie on any mesh face or node during the entire simulation
duration.

The basic sink may be multiplied by any or all of the following quantities

- Fluid relative permeability (when ```use_relative_permeability = true```)
- Fluid mobility ($k_{r} \rho / \nu$)  (when ```use_mobility = true```)
- Fluid mass fraction  (when ```mass_fraction_component``` is specified)
- Fluid enthalpy  (when ```use_enthalpy = true```)
- Fluid internal energy  (when ```use_internal_energy = true```)

That is, $f$ in [eq:line_sink] may be replaced by $fk_{r}$, $f\times\textrm{mobility}$,
etc.  (The units of $fk_{r}$, $f\times\textrm{mobility}$, etc, are kg.s$^{-1}$ for fluid flow, or
J.s$^{-1}$ for heat flow.)  All these additional multiplicative factors are evaluated at the nodal
positions, not at point $x_{i}$, to ensure superior numerical convergence (see
[upwinding](/upwinding.md)).

Examples of these ```use_``` parameters are provided below.  They are
usually used in coupled situations, for instance, a fluid sink may
extract fluid at a given rate, and therefore in a simulation that
includes, the same sink multiplied by fluid enthalpy should be applied
to the temperature variable.

!alert warning
When creating the points for the line sink, it is important to ensure that
every element the line passes through contains at least one (and ideally only
one) point.  When doing this, it is also important to keep in mind that Mesh
displacement and adaptivity can affect the location and number of elements
during the simulation.


!alert warning
When using the PorousFlow Dirac Kernels in conjunction with [`PorousFlowPorosity`](/porous_flow/porosity.md) that depends on volumetric strain (`mechanical = true`) you should set `strain_at_nearest_qp = true` in your GlobalParams block.  This ensures the nodal Porosity Material uses the volumetric strain at the Dirac quadpoint(s).  Otherwise, a nodal Porosity Material evaluated at node $i$ in an element will attempt to use the $i^{th}$ member of volumetric strain, but volumetric strain will only be of size equal to the number of Dirac points in the element.

## Polyline sinks as functions of porepressure and/or temperature

A [`PorousFlowPolyLineSink`](/PorousFlowPolyLineSink.md) is a special case of the general polyline sink.  The function, $f$ in
[eq:line_sink] is assumed to be a *piecewise linear* function of porepressure and/or temperature.
In addition, a multiplication by the line-length associated to $x_{i}$ is also performed.
Specifically:
\begin{equation}
\label{eq:pls}
f(P_{i}, T_{i}) = \frac{1}{2}\left( |x_{i} -
x_{i-1}| + |x_{i} - x_{i+1}| \right) L(P_{i}, T_{i}) \ ,
\end{equation}
where the pre-factor of $L$ is the line-length associated to $x_{i}$,
and $L$ (kg.s$^{-1}$.m$^{-1}$ or J.s$^{-1}$.m$^{-1}$) is a piecewise-linear function, specified by the user in the
MOOSE input file (the weights $w_{i}$ premultiply this $f$ as in
[eq:line_sink] before it is used by MOOSE).

For instance:

!listing modules/porous_flow/test/tests/dirackernels/pls02.i block=DiracKernels

The `PorousFlowPolylineSink` is always accompanied by a [`PorousFlowSumQuantity`](PorousFlowSumQuantity.md) UserObject and often by a [`PorousFlowPlotQuantity`](PorousFlowPlotQuantity.md) Postprocessor

!listing modules/porous_flow/test/tests/dirackernels/pls02.i start=[pls_total_outflow_mass] end=[dictator]

!listing modules/porous_flow/test/tests/dirackernels/pls02.i start=[pls_report] end=[fluid_mass0]

These types of sinks are useful in describing groundwater-surface water interactions via streams and
swamps.  Often a riverbed conductance, measured in kg.Pa$^{-1}$.s$^{-1}$ is defined, which is
\begin{equation}
C = \frac{k_{zz}\rho}{T\mu}L_{\mathrm{seg}}W_{\mathrm{seg}} \ .
\end{equation}
Here $k_{zz}$ is the vertical component of the permeability tensor, $\rho$ is the fluid density,
$\mu$ is the fluid viscosity, and $T$ is a distance variable related to the riverbed thickness.  The
other parameters are $L_{\mathrm{seg}}$ and $W_{\mathrm{seg}}$, which are, respectively, the length
and width of the segment of river that the point $x_{i}$ is representing.  The multiplication by
$L_{\mathrm{seg}}$ is already handled by [eq:pls], and the other terms of $C$ will enter
into the piecewise linear function, $L$.  Three standard types of $L$ are used in groundwater models.

- A perennial stream, where fluid can seep from the porespace to the stream, and vice versa.  Then $L = (C/L_{\mathrm{seg}})(P - P_{\mathrm{atm}})$, where $P_{\mathrm{atm}}$ involves the river stage height;
- An ephemral stream, where fluid can only seep from the porespace to the stream, but not viceversa
  has $L = (C/L_{\mathrm{seg}}) (P-P_{\mathrm{atm}})$ if $P>P_{\mathrm{atm}}$, and zero otherwise.  This is a pure
  sink since $s>0$ always;
- A rate-limited stream, where fluid can exchange between the groundwater and stream, but the rate is
  limited.  This can be modelled by using a piecewise linear $L$ that does not exceed given limits (viz, use one of the above cases, but define the `p_or_t_vals` and `fluxes` to limit the fluxes).

## Peaceman Boreholes

Wellbores are implemented in [`PorousFlowPeacemanBorehole`](/PorousFlowPeacemanBorehole.md)
using the method first described by [!cite](peaceman1983).  Here $f$ is a
special function (measured in kg.s$^{-1}$ in standard units) defined in terms of the pressure at a
point at the wall of the wellbore.

### The wellbore pressure

The wellbore pressure is an input into Peaceman's formula.  For any $x_{i}$ along the wellbore, the wellbore pressure is defined as
\begin{equation}
P_{\mathrm{wellbore}}(x_{i}) = P_{\mathrm{bot}} + \gamma \cdot (x_{i} -
x_{i}^{\mathrm{bot}}) \ .
\end{equation}
Here

- $P_{\mathrm{bot}}$ is an input parameter, `bottom_p_or_t`.  It is the pressure at the bottom of the wellbore.

- $x_{i}^{\mathrm{bot}}$ is the position (a point in 3D) of the bottom of the wellbore.  It is defined to be the *last* point in the `point_file`.

- $\gamma$ is a weight vector pointing downwards (product of fluid density and gravity), `unit_weight`.  This means that $P_{\mathrm{wellbore}}(x_{i})$ will be the pressure at point $x_{i}$ in the wellbore, due to gravitational head.  If these gravitational effects are undesirable, the user may simply specify $\gamma = (0,0,0)$ (`unit_weight = '0 0 0'`).

Although unusual, PorousFlow also allows $P_{\mathrm{wellbore}}$ to represent a temperature, by setting `function_of = temperature`, which is useful for non-fluid models that contain polyline sources/sinks of heat.

### Peaceman's fluid flux

Peaceman writes $f$ as
\begin{equation}
\label{eq:peaceman_f}
f(P_{i}, x_{i}) = W \left|C\right| \frac{k_{\mathrm{r}}\rho}{\mu}(P_{i} - P_{\mathrm{wellbore}})
\end{equation}
Let us discuss each term on the RHS separately.  For boreholes that involve heat only (with `function_of = temperature`) the $P$ in the above expression and the discussion below should be replaced by the temperature.



### The mobility

[eq:peaceman_f] contains  $k_{\mathrm{r}}$, $\rho$, and $\mu$, which are the fluid
relative permeability, density and viscosity.  Hence the term
$k_{\mathrm{r}}\rho/\mu$ is the mobility, so users should choose to
multiply by the mobility (`use_mobility = true`) when using
[`PorousFlowPeacemanBorehole`](/PorousFlowPeacemanBorehole.md).
Recall that all the multiplicative factors, including mobility, are
evaluated at the nodal positions, not the position $x_{i}$.  This
ensures superior numerical convergence.

!alert note
You should almost always set `use_mobility=true`.  The exceptions are when using the volumetric version of PorousFlow (when `multiply_by_density = false` appears in your input file) or in non-fluid simulations (`function_of = temperature`).


### The character

The $C$ in [eq:peaceman_f] is called the `character` of the wellbore.  There are two standard choices (note that $f$ depends only on the absolute value $|C|$).

- With `character = 1` then $C=1$ for $P>P_{\mathrm{wellbore}}$, and zero otherwise.  In this case the wellbore is called a *production* wellbore, and it acts as a sink, removing fluid from the porespace.

- With `character=-1`, $C=-1$ for $P<P_{\mathrm{wellbore}}$, and zero otherwise.  In this case the wellbore is called an *injection* wellbore, and it acts as a source, adding fluid to the porespace.

Generalising, `character` may be chosen to be different from $\pm 1$, which is useful for time-varying borehole strengths.  The above two choices are generalised to:

- if $C>0$ the wellbore is active only for $P>P_{\mathrm{wellbore}}$ (and has zero strength otherwise);

- if $C<0$ the wellbore is active only for $P<P_{\mathrm{wellbore}}$.


### The well constant

The function $W$ is called the `well_constant` of the wellbore, and is measured in units of length$^{3}$.  Usually it is computed automatically by PorousFlow using Peaceman's formulae, below.

Peaceman described the form of $W$ by performing 2D analytical and numerical calculations to obtain the following formula

\begin{equation}
W = 2\pi \sqrt{\kappa_{xx}\kappa_{yy}}L_{z}/\ln(r_{e}/r_{\mathrm{bh}})
\ ,
\end{equation}

In this formula:

- the borehole is oriented along the $z$ direction (this is generalised in PorousFlow: the direction is defined by the plaintext file of points mentioned in [eq:bh_plaintext_format]);

- $\kappa_{xx}$ and $\kappa_{yy}$ are the diagonal components of the permeability tensor in the $(x,y)$ plane (defined by a PorousFlow Material, and generalised in PorousFlow to consider the components normal to the borehole direction);

- $L_{z}$ is the length of the borehole (defined by the plaintext file of points in [eq:bh_plaintext_format]);

- $r_{\mathrm{bh}}$ is the borehole radius (an input parameter, which is encoded in the plaintext file of points as the first entry in each row: see [eq:bh_plaintext_format]);

- $r_{e}$ is the effective borehole radius.

For a cell-centred finite-difference approach, [!cite](peaceman1983) found that
\begin{equation}
r_{e} = 0.28 \frac{\sqrt{\sqrt{\kappa_{xx}/\kappa_{yy}}L_{x}^{2} +
    \sqrt{\kappa_{yy}/\kappa_{xx}}L_{y}^{2}}}{(\kappa_{xx}/\kappa_{yy})^{1/4}
  + (\kappa_{yy}/\kappa_{xx})^{1/4}}
= 0.2 \frac{\sqrt{\frac{1}{2}\sqrt{\kappa_{xx}/\kappa_{yy}}L_{x}^{2} +
    \frac{1}{2}\sqrt{\kappa_{yy}/\kappa_{xx}}L_{y}^{2}}}{\frac{1}{2}(\kappa_{xx}/\kappa_{yy})^{1/4}
  + \frac{1}{2}(\kappa_{yy}/\kappa_{xx})^{1/4}} \ .
\end{equation}
Here $L_{x}$ and $L_{y}$ are the finite-difference spatial sizes.

Other authors have generalised Peaceman's approach to writing $W$ for different geometrical
situations.  Some of these are contained in [!cite](chen2009), where they show that for a finite
element situation with square elements of size $L$, the borehole at a nodal position, and isotropic
permeability

\begin{equation}
r_{e} =  0.113L \ .
\end{equation}

Note that the 0.113 is substantially different to Peaceman's 0.2, demonstrating that this method of
introducing wellbores dependent on the geometry.  The user may specify this quantity via the
`re_constant` input parameter (which defaults to Peaceman's 0.28).


### Simple examples

In the following examples, a vertical borehole is placed through the centre of a single element, and fluid flow to the borehole as a function of porepressure is measured.  The borehole geometry is

!listing modules/porous_flow/test/tests/dirackernels/bh02.bh

meaning that the borehole radius is 0.1m and it is 1m long.

These examples are part of the MOOSE test suite, and they are testing that [eq:peaceman_f] is encoded correctly.  The parameters common to each of these tests are:

!table id=commonParams caption=Common parameters in the tests
| Parameter | Value |
| - | - |
Borehole radius, $r_{\mathrm{bh}}$ | 0.1 m |
Bottomhole pressure, $P_{\mathrm{bot}}$ | 0 |
Gravity | 0 |
Unit fluid weight | 0 |
Element size | $2\times 2\times 2\,$m$^{3}$ |
Isotropoic permeability | $10^{-12}\,$m$^{2}$ |
Fluid reference density | 1000 kg.m$^{-3}$ |
Fluid bulk modulus | 2 GPa |
Fluid viscosity | $10^{-3}\,$Pa.s |
Van Genuchten $\alpha$ | $10^{-5}\,$Pa |
Van Genuchten $m$ | 0.8  |
Residual saturation | 0 |
FLAC relperm $m$ | 2 |

It is remotely possible that the MOOSE implementation *applies* the borehole flux incorrectly, but *records* it as a Postprocessor
correctly as specified by [eq:peaceman_f].  Therefore, these simulations also record the fluid mass and mass-balance error in
order to check that the fluid mass is indeed being correctly changed by the borehole.

A production wellbore:

!listing modules/porous_flow/test/tests/dirackernels/bh02.i block=DiracKernels

with an initially fully-saturated medium yields the correct result ([bh02_flow.fig] and [bh02_error.fig]):

!media bh02_flow.png style=width:40%;margin-left:10px caption=The flow to a production wellbore in MOOSE agrees with the expected result.  id=bh02_flow.fig

!media bh02_error.png style=width:40%;margin-left:10px caption=The mass-balance error is virtually zero.  id=bh02_error.fig

An injection wellbore:

!listing modules/porous_flow/test/tests/dirackernels/bh03.i block=DiracKernels

with a fully-saturated medium yields the correct result ([bh03_flow.fig] and [bh03_error.fig]):

!media bh03_flow.png style=width:40%;margin-left:10px caption=The flow from an injection wellbore in MOOSE agrees with the expected result.  id=bh03_flow.fig

!media bh03_error.png style=width:40%;margin-left:10px caption=The mass-balance error is virtually zero.  id=bh03_error.fig

A production wellbore with bottomhole porepressure $P_{\mathrm{bot}}=-1\,$MPa so it forces desaturation of an initially-saturated medium:

!listing modules/porous_flow/test/tests/dirackernels/bh04.i block=DiracKernels

yields the correct result ([bh04_flow.fig] and [bh04_error.fig]):

!media bh04_flow.png style=width:40%;margin-left:10px caption=The flow to a production wellbore in MOOSE agrees with the expected result.  id=bh04_flow.fig

!media bh04_error.png style=width:40%;margin-left:10px caption=The mass-balance error is virtually zero.  id=bh04_error.fig

An injection wellbore with $P_{\mathrm{bot}}=0$ so it is only active when the rock porepressure is negative:

!listing modules/porous_flow/test/tests/dirackernels/bh05.i block=DiracKernels

yields the correct result for an initially unsaturated medium ([bh05_flow.fig] and [bh05_error.fig]):

!media bh05_flow.png style=width:40%;margin-left:10px caption=The flow from an injection wellbore in MOOSE agrees with the expected result.  id=bh05_flow.fig

!media bh05_error.png style=width:40%;margin-left:10px caption=The mass-balance error is virtually zero.  id=bh05_error.fig

Each of these record the total fluid flux (kg) injected by or produced by the borehole in a [`PorousFlowSumQuantity`](PorousFlowSumQuantity.md) UserObject and outputs this result using a [`PorousFlowPlotQuantity`](PorousFlowPlotQuantity.md) Postprocessor:

!listing modules/porous_flow/test/tests/dirackernels/bh02.i start=[borehole_total_outflow_mass] end=[dictator]

!listing modules/porous_flow/test/tests/dirackernels/bh02.i start=[bh_report] end=[fluid_mass0]


### Reproducing the steady-state 2D analytical solution

The PorousFlow fluid equation (see [governing equations](governing_equations.md)) for a fully-saturated medium with $\rho \propto
\exp(P/B)$ and large constant bulk modulus $B$ becomes Darcy's equation
\begin{equation}
\frac{\partial}{\partial t}\rho =  \nabla_{i}\alpha_{ij}\nabla_{j}\rho
\end{equation}
where $\alpha_{ij} = k_{ij}B/(\mu\phi)$, with notation described
in the [nomenclature](nomenclature.md).   In the isotropic case (where $k_{ij} =
\kappa \delta_{ij}$), the steadystate equation is just Laplace's
equation
\begin{equation}
\nabla^{2}\rho = 0 \ ,
\end{equation}

Place a borehole of radius $r_{\mathrm{bh}}$ and infinite length
oriented along the $z$ axis.  Then the situation becomes 2D and can be
solved in cylindrical coordinates, with $\rho=\rho(r,\theta)$ and
independent of $z$.  If the pressure at the borehole wall
$r=r_{\mathrm{bh}}$ is $P_{\mathrm{bh}}$, then the fluid density is
$\rho_{\mathrm{bh}} \propto \exp(P_{\mathrm{bh}}/B)$.  Assume that at
$r=R$ the fluid pressure is held fixed at $P_{R}$, or equivalently the
density is held fixed at $\rho_{R}$.  Then the solution of Laplace's
equation is well-known to be
\begin{equation}
\label{eq:log_bh}
\rho = \rho_{\mathrm{bh}} + (\rho_{R} - \rho_{\mathrm{bh}})
\frac{\log(r/r_{\mathrm{bh}})}{\log(R/r_{\mathrm{bh}})} \ .
\end{equation}
This is the fundamental solution used by Peaceman and others to derive
expressions for $W$ by comparing with numerical expressions resulting
from [eq:peaceman_f].

Chen and Zhang [!cite](chen2009) have derived an expression for $W$
in the case where this borehole is placed at a node in a square mesh.
This test compares the MOOSE steadystate solution with a single
borehole with $W$ defined by
Chen and Zhang's formula (specifically, the `re_constant` needs to be changed from its default value) is compared with [eq:log_bh] to
illustrate that the MOOSE implementation of a borehole is correct.

The `PorousFlowPeacemanBorehole` is:

!listing modules/porous_flow/test/tests/dirackernels/bh07.i block=DiracKernels

The mesh used is shown in [bh07_mesh.fig].

!media bh07_mesh.png style=width:50%;margin-left:10px caption=The mesh used in the comparison with [eq:log_bh], with the green dot indicating the position of the borehole.  The central elements are $10\times 10\,$m$^{2}$, and the outer boundary is at radius 300m.  id=bh07_mesh.fig

[bh07.fig] show the comparison between the MOOSE result and [eq:log_bh].  Most parameters in this
study are identical to those given in the [commonParams] with the
following exceptions: the mesh is shown in [bh07_mesh.fig];
the permeability is $10^{-11}\,$m$^{2}$; the borehole radius is 1 m;
the borehole pressure is $P_{\mathrm{bh}}=0$; the outer radius is
$r=300\,$m; and the outer pressure is $P_{R}=10\,$MPa.

!media bh07.png style=width:50%;margin-left:10px caption=Comparison of the MOOSE results (dots) with the analytical solution [eq:log_bh] for the steadystate porepressure distribution surrounding single borehole.  id=bh07.fig


### Injecting and producing in thermo-hydro simulations

The `PorousFlowPeacemanBorehole` may be used to model injection and production in thermo-hydro simulations.  Suppose that cool water is being injected into an initially hot reservoir via a vertical injection well, and that water is being removed from the system via a vertical production well.  In this example we shall study an essentially 2D situation without gravity with parameters defined in [inj_prod_params]

!table id=inj_prod_params caption=Injection-production parameters
| Parameter | Value |
| - | - |
Injection borehole radius | 0.2 m |
Injection borehole vertical length  | 10 m |
Injection bottomhole pressure | 21 MPa |
Injection temperature | 300 K |
Production borehole radius | 0.25 m |
Production borehole vertical length  | 10 m |
Production bottomhole pressure | 20 MPa |
Injection-production well separation | 30 m |
Reservoir initial porepressure | 20 MPa |
Reservoir initial temperature | 400 K |
Gravity | 0 |
Unit fluid weight | 0 |

The fluid properties are defined as:

!listing modules/porous_flow/test/tests/dirackernels/injection_production.i block=FluidProperties

The fluid injection and production is implemented in a way that is now familiar:

!listing modules/porous_flow/test/tests/dirackernels/injection_production.i start=[fluid_injection] end=[remove_heat_at_production_well]

The injection temperature is set via:

!listing modules/porous_flow/test/tests/dirackernels/injection_production.i block=BCs

Alternatively, the heat energy of the injected fluid could be worked out and injected into the heat-conservation equation via a borehole.

The production of fluid means that heat energy must be removed at exactly the rate associated with the fluid-mass removal.  This is implemented by using an identical `PorousFlowPeacemanBorehole` to the fluid production situation *but* associating it with the temperature variable and with `use_enthalpy = true`:

!listing modules/porous_flow/test/tests/dirackernels/injection_production.i start=[remove_heat_at_production_well] end=[]

Here the heat energy per timestep is saved into a [`PorousFlowSumQuantity`](PorousFlowSumQuantity.md) UserObject which may be extracted using a [`PorousFlowPlotQuantity`](PorousFlowPlotQuantity.md) Postprocessor, which would be an important quantity in a geothermal application:

!listing modules/porous_flow/test/tests/dirackernels/injection_production.i block=Postprocessors


Scaling of the variables ensures [good convergence](convergence.md):

!listing modules/porous_flow/test/tests/dirackernels/injection_production.i block=Variables



Some results (run on a fine mesh) are shown in [inj_prod_p.fig] and [inj_prod_t.fig].

!media injection_production_P.png style=width:50%;margin-left:10px caption=Porepressure after $2\times 10^{6}\,$s of injection and production. The black spots indicate the wells.  id=inj_prod_p.fig

!media injection_production_T.png style=width:50%;margin-left:10px caption=Temperature after $2\times 10^{6}\,$s of injection and production.  the black spots indicate the wells.  id=inj_prod_t.fig




!bibtex bibliography
