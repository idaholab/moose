# LayeredSideDiffusiveFluxAverage

!syntax description /UserObjects/LayeredSideDiffusiveFluxAverage

The diffusive flux $\phi$ is computed with the following equation:

!equation
\phi = \int_{\partial \Omega} D(\vec{r}) \nabla u(\vec{r}) \cdot \vec{n}(\vec{r}) d\partial \Omega

where $\partial \Omega$ is the boundary of integration, $D$ the diffusion coefficient, $u$ the variable
of interest, $\vec{n}$ the normal to the boundary, and $\vec{r}$ indicates the position.

## Example Input File Syntax

!listing test/tests/userobjects/layered_side_integral/layered_side_diffusive_flux_average.i id=example_id block=UserObjects
         caption=Example input file snippet with a LayeredSideDiffusiveFluxAverage.

!syntax parameters /UserObjects/LayeredSideDiffusiveFluxAverage

!syntax inputs /UserObjects/LayeredSideDiffusiveFluxAverage

!syntax children /UserObjects/LayeredSideDiffusiveFluxAverage
