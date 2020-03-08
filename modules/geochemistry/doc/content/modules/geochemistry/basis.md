# The chemical basis

Notation and definitions are described in [geochemistry_nomenclature.md].

Choosing the "basis" of primary species is the first stage in a geochemical simulation.  Throughout this page we use the standard geochemical notation, following [!cite](bethke_2007), in which *the subscript gives meaning to the quantity*.  For instance, $A_{i}$ is a primary species, while $A_{l}$ is a mineral.  The "$i$" and "$l$" have given meaning to the $A$.

## Default basis

Reading the [reaction database](database.md) provides a default list of basis species.  H$_{2}$O is always present in this basis, along with many other species such as Ag$^{+}$, Al$^{3+}$, Fe$^{2+}$, H$^{+}$, HCO$_{3}^{-}$, etc.

Denote water by $A_{w}$ and all the other species in the default basis by $A_{i}$.  These are convenient labels: they are not quantities such as number of moles of a substance.  For example, one of the $A_{i}$ will be Ag$^{+}$, another will be Al$^{3+}$, etc.  At this stage
\begin{equation}
\mathrm{basis} = (A_{w}, A_{i})
\end{equation}

## Redox disequilibrium

The [database](database.md) also contains a number of "redox couples".  These represent the basis species (e.g. Fe$^{2+}$) in alternative oxidataion states (e.g. Fe$^{3+}$).  The user must define which redox species are "coupled" and which are "decoupled".

- Coupled (the default).  In this case, the redox pair is in equilibrium.  The database may be used to express the alternative oxidation state in terms of basis species.  For instance: $\mathrm{Fe}^{3+}= -0.5\mathrm{H}_{2}\mathrm{O} + \mathrm{Fe}^{2+} + \mathrm{H}^{+} + 0.25\mathrm{O}_{2}\mathrm{(aq)}$, which allows Fe$^{3+}$ to be eliminated from all reactions (similar to a [swap](swap.md)).  The alternative oxidation state may thereby be considered to be a secondary species (with its own equilibrium constant) and will not appear in the basis.

- Decoupled.  In this case, the redox pair is in disequilibrium.  There are two possibilities.  (A) The alternative oxidation state (e.g. Fe$^{3+}$) is added to the basis as a primary species, and its redox reactions specified in the database is ignored.  This means it is necessary to specify an initial condition for it, and solve for its concentration.  (B) The redox reaction is modelled using a [kinetic approach](kinetics.md).  In this case, the alternative oxidataion state is not added to the basis, but a reaction is specified that produces it, along with a reaction rate law.

After choosing which redox pairs are decoupled, and adding the non-kinetically-controlled alternative oxidation states to the basis, it is
\begin{equation}
\mathrm{basis} = (A_{w}, A_{i}) \ .
\end{equation}
The $A_{i}$ contains the original basis species and the decoupled redox pairs that are not controlled by a kinetic law.

## Minerals

The [database](database.md) also contains information concerning mineralisation reactions.  It may be convenient to remove a number of the $A_{i}$ in favour of an equal number of minerals.  This is performed via a [swap](swap.md).  This is the only way of specifying a particular concentration for a mineral.  Often minerals are thought to be immobile (with regards to [transport](transport.md)) although this assumption might be relaxed to account, for example, for the migration of colloids or suspended sediment.

Of course, the basis species Pb$^{2+}$ cannot be sensibly replaced by the mineral Acanthite (Ag$_{2}$S), but Anglesite, PbSO$_{4}$ would be appropriate.

After including the desirable minerals, the basis is
\begin{equation}
\mathrm{basis} = (A_{w}, A_{i}, A_{k}) \ .
\end{equation}
The $A_{i}$ contains the original basis species and the decoupled redox pairs that are not controlled by a kinetic law, minus those removed by inclusion of minerals.  The $A_{k}$ are the included minerals.

## Gases

The [database](database.md) also contains reactions involving gases.  It may be convenient to remove a number of the $A_{i}$ in favour of an equal number of gases, because the [fugacities](fugacity.md) of the gases are known.  This is performed via a [swap](swap.md).  This is the only way of specifying a particular fugacity for a gas.

Of course, the basis species Pb$^{2+}$ cannot be sensibly replaced by the gas CO$_{2}$(g), but, given the reaction
\begin{equation}
\mathrm{CO}_{2}\mathrm{(g)} \rightleftharpoons -\mathrm{H}_{2}\mathrm{O} + \mathrm{H}^{+} + \mathrm{HCO}_{3}^{-} \ ,
\end{equation}
the gas CO$_{2}$(g) could replace either H$^{+}$ or HCO$_{3}^{-}$.

After including the desirable gases, the basis is
\begin{equation}
\mathrm{basis} = (A_{w}, A_{i}, A_{k}, A_{m}) \ .
\end{equation}
The $A_{i}$ contains the original basis species and the decoupled redox pairs that are not controlled by a kinetic law, minus those removed by inclusion of minerals and gases.  The $A_{k}$ are the included minerals, while the $A_{m}$ are the included gases.


## Sorbing sites

The Langmuir approach to sorption is that the rock matrix consists of a number of sorbing sites.  Sorbing species compete to occupy these sites, and once sorbed they do not flow via any transport.  New "sorbed" species are introduced, and equilibrium equations involving these and the remainder of the basis are written.  To include the sorbing site, the basis is expanded:
\begin{equation}
\mathrm{basis} = (A_{w}, A_{i}, A_{k}, A_{m}, A_{p}) \ .
\end{equation}
where $A_{p}$ is a single new entry.  It has a molality of unoccupied sites, $m_{p}$ and a total mole number of sorbing sites, $M_{p}$ in the same way that each of the other basis species has a molality and total mole number.

The ion-exchange approach to sorption is similar: a new basis species representing exchange sites is introduced.  The mass action is similar to the Langmuir mass action but involves the electric charge of the ions involved in the exchange.

## Surface complexation

The above models of sorption do not account for the electrical state of the porous-skeleton surface, which varies sharply with pH, ionic strength and solution composition.  To model this, an extra entry, $A_{p}$ for *each type* of surface site may be included in the basis:
\begin{equation}
\mathrm{basis} = (A_{w}, A_{i}, A_{k}, A_{m}, A_{p}) \ .
\end{equation}
(Now there are multiple $A_{p}$: one for each surface complex.)

Reactions to form each surface complex (i.e. species made out of the basis that form on the porous-skeleton's surface) are written, and the mass-action involves a surface potential, which is supplied by the user.

!bibtex bibliography
