[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_12.md) |

# Porous Flow Tutorial Page 13.  More elaborate chemistry

A very simple chemical system was built in [Page 07](porous_flow/tutorial_07.md).  The reader is encouraged to consult that page before moving to the more elaborate situation described here.  This page:

- does not use an `Action` to describe the Kernels, Materials, etc.  This makes the input file quite long, but perhaps more easy to extend to multi-phase situations, different boundary conditions, etc.  An `Action` could easily be used instead, by copying the chemistry from this tutorial to that on [Page 07](porous_flow/tutorial_07.md).
- builds a fully-saturated aqueous chemical system that could be used to describe dolomite precipitation and dissolution.

This page illustrates that it is possible to build quite complicated chemical systems within PorousFlow.  However, geochemists will recognize these are still unrealistically simple, since they contain only a few species, the activity coefficients are all unity, the equilibrium constants are not easily temperature-dependent, etc.  If you require state-of-the-art geochemical modelling capability, please use MOOSE's Geochemistry module.

## The equilibrium system

The equilibrium system has:

- 5 primary species, which are H$^{+}$, HCO$_{3}^{-}$, Ca$^{2+}$, Mg$^{2+}$, Fe$^{2+}$.
- 5 secondary species, which are CO$_{2}$(aq), CO$_{3}^{2-}$, CaHCO$_{3}^{+}$, MgHCO$_{3}^{+}$, FeHCO$_{3}^{+}$.

The equations are
\begin{equation}
\begin{aligned}
H^+ + HCO_3^- &\rightleftharpoons CO_2(aq)  & K_{eq} &= 10^{6.341} \\
HCO_3^- - H^{+} &\rightleftharpoons CO_3^{2-} & K_{eq} &= 10^{-10.325} \\
Ca^{2+} + HCO_3^- &\rightleftharpoons CaHCO_3^+ & K_{eq} &= 10^{-0.653} \\
Mg^{2+} + HCO_3^- &\rightleftharpoons MgHCO_3^+ & K_{eq} &= 10^{-2} \\
Fe^{2+} + HCO_3^- &\rightleftharpoons FeHCO_3^+ & K_{eq} &= 10^{-3} \\
\end{aligned}
\end{equation}
Some of these equilibrium constants have been chosen rather arbitrarily.

The primary species are represented as PorousFlow variables:

!listing modules/porous_flow/examples/tutorial/13.i start=[Variables] end=[AuxVariables]

The equilibrium reactions are encoded into this Material:

!listing modules/porous_flow/examples/tutorial/13.i start=[equilibrium_massfrac] end=[kinetic]



## The kinetic system

This is
\begin{equation}
Ca^{2+} + 0.8 Mg^{2+} + 0.2 Fe^{2+} + 2 HCO_{3}^{-} - 2 H^{+} \rightleftharpoons CaMg_{0.8}Fe_{0.2}(CO_{3})_{2} \ ,
\end{equation}
with the following parameters:

- molar volume 64365.0$\,$L(solution)/mol,
- mineral density 2875.0$\,$kg(precipitate)/m$^{3}$(precipitate)
- equilibrium constant $K_{eq} = 10^{2.5135}$,
- specific reactive surface area $1.2\times 10^{-8}\,$m$^{2}$/L,
- kinetic rate constant $k_{m} = 3\times 10^{-4}\,$mol/m$^{2}$/s,
- activation energy $E_{a} = 15000\,$J/mol,
- the primary activity coefficients are all unity,
- and the $\eta$ and $\theta$ exponents are also unity.

Some of these quantities have been chosen rather arbitrarily.  This kinetic system is encoded in the input file as:

!listing modules/porous_flow/examples/tutorial/13.i start=[kinetic] end=[dolomite_conc]



## Geometry

The model is just a 1D line, extending between $x=0$ and $x=1$.

!listing modules/porous_flow/examples/tutorial/13.i start=[Mesh] end=[Variables]

## The initial and boundary conditions

### Primary variables

Each of the primary variables are initialised to have concentration $10^{-6}\,$m$^{3}$(species)/m$^{3}$(solution) everywhere in the domain except for at the left-hand side ($x=0$) where they have concentration $0.05$.  The boundary conditions are to fix these values at the left and right sides of the domain.  For instance:

!listing modules/porous_flow/examples/tutorial/13.i start=[h+_ic] end=[hco3_ic]

and

!listing modules/porous_flow/examples/tutorial/13.i start=[h+_left] end=[ca2+_left]

!listing modules/porous_flow/examples/tutorial/13.i start=[h+_right] end=[ca2+_right]

*Please remember* that boundary conditions in PorousFlow are usually more complicated than setting Dirichlet or Preset boundary conditions: see [boundary conditions](boundaries.md).  Looking at the results below you can clearly see the effect of the naive boundary conditions placed on the right-hand side.

### Dolomite

The initial condition for dolomite is $10^{-7}\,$m$^{3}$(precipitate)/m$^{3}$(porous material).  This is implemented in the Auxillary system by

!listing modules/porous_flow/examples/tutorial/13.i start=[AuxVariables] end=[GlobalParams]

and the Material:

!listing modules/porous_flow/examples/tutorial/13.i start=[dolomite_conc] end=[simple_fluid]

Given the above equilibrium constant and concentrations of the primary species, the dolomite immediately begins to dissolve into solution.

### Porepressure

The porepressure is fixed to have gradient $dP/dx = 10^{6}\,$Pa/m.

!listing modules/porous_flow/examples/tutorial/13.i start=[pressure_ic] end=[h+_ic]

With a permeability of $10^{-7}\,$m$^{2}$ and a fluid viscosity of $10^{-3}\,$Pa.s, the Darcy velocity is $0.1\,$m/s.  The porosity is held fixed at 0.2.

## Results

!media tut13_conc.png style=width:50%;margin-left:10px caption=Two of the primary species concentrations at the end of the simulation.  id=tut13_conc.fig

!media tut13_dolo.png style=width:50%;margin-left:10px caption=The precipitated dolomite concentration at the end of the simulation.  id=tut13_dolo.fig

[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_12.md) |
