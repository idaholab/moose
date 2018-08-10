# GeneralVaporMixtureFluidProperties

!syntax description /Modules/FluidProperties/GeneralVaporMixtureFluidProperties

This class computes fluid properties for arbitrary vapor mixtures.
This model assumes that the gases occupy separate volumes, but share the same
pressure $p$ and temperature $T$. Specific volume and specific
internal energy have the following mixture relations, where $i$ denotes
the index of the gas in the mixture, and the lack of a subscript denotes the
mixture quantity:
\begin{equation}
 v = \sum\limits_i^N x_i v_i(p, T) ,
\end{equation}
\begin{equation}
 e = \sum\limits_i^N x_i e_i(p, T) ,
\end{equation}
where $N$ is the number of gases present in the mixture,
and $v_i(p, T)$ and $e_i(p, T)$ denote the equation-of-state calls
from the respective gas from $p$ and $T$. Therefore, if $v$ and
$e$ are known, then to get $p$ and $T$, a size-2 nonlinear system
is solved.

!syntax parameters /Modules/FluidProperties/GeneralVaporMixtureFluidProperties

!syntax inputs /Modules/FluidProperties/GeneralVaporMixtureFluidProperties

!syntax children /Modules/FluidProperties/GeneralVaporMixtureFluidProperties
