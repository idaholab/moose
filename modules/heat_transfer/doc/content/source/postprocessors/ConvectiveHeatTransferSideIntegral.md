# ConvectiveHeatTransferSideIntegral

!syntax description /Postprocessors/ConvectiveHeatTransferSideIntegral

## Description

This postprocessor computes the total heat flux $Q$ by integrating
the product of the temperature difference and the heat transfer coefficient
over a set of boundaries.

\begin{equation}
 Q = \int\limits_{S} h (T_s - T_f)  dS,
\end{equation}

where $S$ is the set of boundaries, $T_s$ is the solid temperature, $T_f$ is the fluid temperature, and $h$ is the heat transfer coefficient.

This postprocessor is useful for ensuring conservative transfers when Robin boundary conditions are used.


## Example Input File Syntax

!listing modules/heat_conduction/test/tests/postprocessors/convective_ht_side_integral.i
block=Postprocessors


!syntax parameters /Postprocessors/ConvectiveHeatTransferSideIntegral

!syntax inputs /Postprocessors/ConvectiveHeatTransferSideIntegral

!syntax children /Postprocessors/ConvectiveHeatTransferSideIntegral

!bibtex bibliography
