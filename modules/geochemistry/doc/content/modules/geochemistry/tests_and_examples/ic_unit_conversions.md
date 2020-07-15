# Converting measures of concentration

The chemical composition of waters may be specified in many different ways.  This page discusses a few of these.

## Molal

This means moles per kg of solvent water.  Note that the mass of solvent water will typically be different than the mass of the solution, since the latter includes solutes.

## `moles_bulk_species`

This is one the accepted inputs of `geochemistry` and is the total number of moles of a species.  It includes the "free" contribution, which is the number of moles of a species that is "floating around" in the solution, as well as the number of moles that appears in other species.  For example, if a solution contains 2 free moles of Cl$^{-}$, and 1 mole of the complex NaCl, then the total bulk number of moles of Cl$^{-1}$ is 3.

## g/kg or mg/kg

This measures the bulk mass (in g or mg) of a species per kg of solution.  The solution's mass (the denominator) is a sum of the solvent-water's mass and the mass of the solutes.  A useful conversion is
\begin{equation}
\mathrm{moles} = \frac{\mathrm{Concentration} \times \mathrm{Mass}_{\mathrm{solution}}}{\mathrm{MolarMass}} \ ,
\end{equation}
where the concentration is specified in g.kg$^{-1}$, the mass of the solution (including solvent water and solutes), and the molar mass of the species has units g.mol$^{-1}$.  The mass of the solution can be worked out from the TDS (below).

## TDS

The definition of TDS is
\begin{equation}
\mathrm{TDS} = \frac{\mathrm{Mass}_{\mathrm{solutes}}}{\mathrm{Mass}_{\mathrm{solventWater}} + \mathrm{Mass}_{\mathrm{solutes}}} = \frac{\mathrm{Mass}_{\mathrm{solutes}}}{\mathrm{Mass}_{\mathrm{solution}}} \ .
\end{equation}
TDS is usually measured in mg/kg or g/kg.
