# IdealRealGasMixtureFluidProperties

!syntax description /FluidProperties/IdealRealGasMixtureFluidProperties

This class computes fluid properties for gaseous mixtures with a condensable
(primary) component and non-condensable (secondary) components.
This model assumes that each gas in the mixture occupies the entire mixture
volume at the temperature $T$ and the partial pressure $p_i$. Pressure and
specific internal energy have the following mixture relations, where $i$
denotes the index of the gas in the mixture, and the lack of a subscript denotes
the mixture quantity:
\begin{equation}
 p = \sum\limits_i^N p_i(T, v_i) ,
\end{equation}
\begin{equation}
 e = \sum\limits_i^N x_i e_i(T, v_i) ,
\end{equation}
where $N$ is the number of gases present in the mixture
and $v_i = v / x_i$. p_i(T, v_i) and $e_i(T, v_i)$ denote the equation-of-state
calls from the respective gas from $T$ and the partial specific volume $v_i$.
Therefore, if $v$ and $e$ are known, then to get $p$ and $T$, the nonlinear
equation $e(T, v)$ is solved for $T$ first. Then $p$ is calculated from 
$p(T, v)$.

!syntax parameters /FluidProperties/IdealRealGasMixtureFluidProperties

!syntax inputs /FluidProperties/IdealRealGasMixtureFluidProperties

!syntax children /FluidProperties/IdealRealGasMixtureFluidProperties
