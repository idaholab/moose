# The chemical basis

Notation and definitions are described in [geochemistry_nomenclature.md].

Choosing the "basis" of primary species is the first stage in a geochemical simulation.  Throughout this page we use the standard geochemical notation, following [!cite](bethke_2007), in which *the subscript gives meaning to the quantity*.  For instance, $A_{i}$ is a primary species, while $A_{l}$ is a mineral.  The "$i$" and "$l$" have given meaning to the $A$.

The basis is a minimal, independent set of chemical component.  This is a complete basis in the mathematical sense: all other chemical components can be expressed in terms of the basis, via equilibrium or kinetic chemical reactions; and none of the basis components can be expressed in terms of the other basis components.  For example, a very simple basis is $\left\{ \mathrm{H}_{2}\mathrm{O}, \mathrm{H}^{+} \right\}$.  A secondary species that can be expressed in this basis is OH$^{-}$, through the equilibrium reaction $\mathrm{OH}^{-} \rightleftharpoons \mathrm{H}_{2}\mathrm{O} - \mathrm{H}^{+}$.  In practice, the basis is often set equal to the chemicals experimentally measured in a water sample.

It is common for geochemical-solving algorithms to change the basis during the solving procedure, for various reasons, such as the concentration of a basis species becoming very small.  Using the above example, H$^{+}$ may be [swapped out](swap.md) of the basis and OH$^{-}$ "swapped in", so the new basis is $\{\mathrm{H}_{2}\mathrm{O}, \mathrm{OH}^{-}\}$, and the equilibrium reaction becomes H$^{+} \rightleftharpoons \mathrm{H}_{2}\mathrm{O} - \mathrm{OH}^{-}$.

## Default basis

Reading the [reaction database](geochemistry/database/index.md) provides a default list of basis species.  H$_{2}$O is always present in this basis, potentially along with many other species such as Ag$^{+}$, Al$^{3+}$, Fe$^{2+}$, H$^{+}$, HCO$_{3}^{-}$, etc.

Denote water by $A_{w}$ and all the other species in the default basis by $A_{i}$.  These are convenient labels: they are not quantities such as number of moles of a substance.  For example, one of the $A_{i}$ will be Ag$^{+}$, another will be Al$^{3+}$, etc.  At this stage
\begin{equation}
\mathrm{basis} = \{A_{w}, A_{i}\}
\end{equation}

## Equilibrium species

The geochemical database contains a large list of species that can be at equilibrium with the basis species, such as NaCl, OH$^{-}$, etc.  The database prescribes each equilibrium species with a reaction and an equilibrium constant for that reaction.  They are not in the default basis, but may be [swapped](swap.md) with one of the default basis species if convenient, for instance, because an experimental observation has measured the molality of an equilibrium species.

After performing any swaps, the basis is
\begin{equation}
\mathrm{basis} = \{A_{w}, A_{i}\} \ ,
\end{equation}
where the $A_{i}$ can potentially contain species that were labelled as equilibrium species in the database.


## Redox disequilibrium

The [database](geochemistry/database/index.md) also contains a number of "redox couples".  These represent the basis species (e.g. Fe$^{2+}$) in alternative oxidataion states (e.g. Fe$^{3+}$).  The user must define which redox species are "coupled" and which are "decoupled".

- Coupled (the default).  In this case, the redox pair is in equilibrium.  The database may be used to express the alternative oxidation state in terms of basis species.  For instance: $\mathrm{Fe}^{3+}= -0.5\mathrm{H}_{2}\mathrm{O} + \mathrm{Fe}^{2+} + \mathrm{H}^{+} + 0.25\mathrm{O}_{2}\mathrm{(aq)}$, which allows Fe$^{3+}$ to be eliminated from all reactions (similar to a [swap](swap.md)) in favor of the basis species H$_{2}$O, Fe$^{2+}$, H$^{+}$ and O$_{2}$(aq).  The alternative oxidation state may thereby be considered to be a secondary species (with its own equilibrium constant) and will not appear in the basis.

- Decoupled.  In this case, the redox pair is in disequilibrium.  There are two possibilities.  (A) The alternative oxidation state (e.g. Fe$^{3+}$) is added to the basis as a primary species, and its redox reactions specified in the database is ignored.  This means it is necessary to specify an initial condition for it, and solve for its concentration.  (B) The redox reaction is modelled using a [kinetic approach](theory/index.md).  In this case, the alternative oxidataion state is not added to the basis, but a reaction is specified that produces it, along with a reaction rate law.

After choosing which redox pairs are decoupled, and adding the non-kinetically-controlled alternative oxidation states to the basis, it is
\begin{equation}
\mathrm{basis} = \{A_{w}, A_{i}\} \ .
\end{equation}
The $A_{i}$ contains the original basis species, and equilibrium species via swaps, and the decoupled redox pairs that are not controlled by a kinetic law.

## Minerals

The [database](geochemistry/database/index.md) also contains information concerning mineralisation reactions.  Mineralisation can be governed by equilibrium mass-balance or kinetics.  If some of the minerals exist as precipitates in the aqueous solution, and they are at equilibrium with the solution (not governed by a kinetic law), then some default basis species are removed in favour of such minerals.  This is performed via a [swap](swap.md).  This is the only way of specifying a particular concentration for a mineral.  Often minerals are thought to be immobile (with regards to [transport](theory/index.md)) although this assumption might be relaxed to account, for example, for the migration of colloids or suspended sediment.

Of course, the basis species Pb$^{2+}$ cannot be sensibly replaced by the mineral Acanthite (Ag$_{2}$S), but Anglesite, PbSO$_{4}$ would be appropriate.

After including the desirable minerals, the basis is
\begin{equation}
\mathrm{basis} = \{A_{w}, A_{i}, A_{k}\} \ .
\end{equation}
The $A_{i}$ contains the original basis species (and potentially some equilibrium species that have been swapped in) and the decoupled redox pairs that are not controlled by a kinetic law, those removed by inclusion of minerals.  The $A_{k}$ are the included minerals.

## Gases

The [database](geochemistry/database/index.md) also contains reactions involving gases.  If the [fugacities](fugacity.md) of the gases are known, then some default basis species are removed in favour of these gases.  This is performed via a [swap](swap.md).  This is the only way of specifying a particular fugacity for a gas.

Of course, the basis species Pb$^{2+}$ cannot be sensibly replaced by the gas CO$_{2}$(g), but, given the reaction
\begin{equation}
\mathrm{CO}_{2}\mathrm{(g)} \rightleftharpoons -\mathrm{H}_{2}\mathrm{O} + \mathrm{H}^{+} + \mathrm{HCO}_{3}^{-} \ ,
\end{equation}
the gas CO$_{2}$(g) could replace either H$^{+}$ or HCO$_{3}^{-}$.

After including the desirable gases, the basis is
\begin{equation}
\mathrm{basis} = \{A_{w}, A_{i}, A_{k}, A_{m}\} \ .
\end{equation}
The $A_{i}$ contains the original basis species (and potentially some equilibrium species that have been swapped in) and the decoupled redox pairs that are not controlled by a kinetic law, minus those removed by inclusion of minerals and gases.  The $A_{k}$ are the included minerals, while the $A_{m}$ are the included gases.


## Sorption and surface complexation

Some components may adsorb onto "sorbing sites" on mineral surfaces, either via an equilibrium reaction or a kinetically-controlled reaction.  The adsorbed species are called surface complexes.

The Langmuir approach to sorption is that a particular mineral has a number of sorbing sites, and the basis is expanded to include a single extra entry representing these sorbing sites.  All sorbing species compete to adsorb onto this single type of site, and once adsorbed they do not flow via any transport.  New "sorbed" species are introduced, $A_{q}$, and equilibrium equations involving these and the remainder of the basis are written.  To include the sorbing site, the basis is expanded:
\begin{equation}
\mathrm{basis} = \{A_{w}, A_{i}, A_{k}, A_{m}, A_{p}\} \ .
\end{equation}
where $A_{p}$ is a single new entry.  It has a molality of unoccupied sites, $m_{p}$ and a total mole number of sorbing sites, $M_{p}$ in the same way that each of the other basis species has a molality and total mole number.

More sophisticated approaches to surface complexation account for the electrical state of the porous-skeleton surface, which varies sharply with pH, ionic strength and solution composition.  To model this, an extra entry, $A_{p}$ for *each type* of surface site may be included in the basis:
\begin{equation}
\mathrm{basis} = \{A_{w}, A_{i}, A_{k}, A_{m}, A_{p}\} \ .
\end{equation}
(Now there are multiple $A_{p}$: one for each surface-site type.)  Each surface complex adsorbs to a specified type of surface site, so that if more than one surface complex is associated with a single type of surface site, they will compete to occupy that site.  There may be more than one type of surface site for each mineral.  Reactions to form each surface complex (i.e. species made out of the basis that form on the porous-skeleton's surface) are written, and the mass-action involves a surface potential, which is supplied by the user.


!bibtex bibliography
