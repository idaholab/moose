# 3DC cohesive law

!syntax description /UserObjects/CohesiveLaw_3DC

This UserObject implement the 3DC Cohesive law model proposed by [cite:Salehani2018].

The 3DC models assume that the traction is function of the displacement jump between
the master and slave interface:

\begin{equation}
[u] =u^{slave} - u^{master}
\end{equation}
Where the master surface is the one in which the interface kernel is defined.

The surface tractions are defined as follow:

\begin{equation}
T_i = \frac{\phi_i}{\delta^{0}_{i}} \frac{[u]_{i} }{\delta^{0}_{i}} exp\left[-\sum_{j=1}^{d}\left(\frac{[u]_{j}}{\delta^{0}_{j}}\right)^{\alpha}\right]
\end{equation}

where $d$ is the dimensionality of the problem, $i=[1,d]$ an $j$ are indexes identifying one of directions, $\alpha=1$ for the opening mode($j=1$) and $\alpha=2$ the sliding($j=2,3$) modes, $delta^{0}$ is the displacement jump associated to the maximum allowable traction. $\phi$ is the potential and is defined as follows:

\begin{equation}
\phi_{i} = \sigma_{i}^{max}\lambda_i\delta_{i}^{0}
\end{equation}

where $\sigma_{i}^{max}$ is the maximum allowable traction on the $i-th$ direction, and $\lambda_1=e$ and $\lambda_{2,3}=\sqrt{2e}$




## Example Input File Syntax

The input parameter `DeltaU0` is a vector containing the value of $\delta^{0}$ at which the maximum
allowable traction occurs. The first and second component represents the normal ($\delta^{0}_1$) and tangential ($\delta^{0}_{2,3}$) separation, respectively.

The input parameter `MaxAllowableTraction` is a vector containing the value of $\sigma^{max}$. The first and second component represents values for normal ($\sigma_1^{max}$) and tangential ($\sigma_{2,3}^{max}$) maximum allowable traction, respectively.


### Single interface

!listing modules/tensor_mechanics/test/tests/cohesive_zone_IK/2D/czmTest3DC_CohesiveLaw2D.i block=UserObjects

### Multiple interfaces

When `split_interface=true` the cohesive interface is split by block pairs:

!listing modules/tensor_mechanics/test/tests/cohesive_zone_IK/2D/czmTest3DC_CohesiveLaw2D_splitInterface.i block=UserObjects



!syntax parameters /UserObjects/CohesiveLaw_3DC

!syntax inputs /UserObjects/CohesiveLaw_3DC




!bibtex bibliography
