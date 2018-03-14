# KineticDisPreRateAux

!syntax description /AuxKernels/KineticDisPreRateAux

Calculates the kinetic reaction rate based on transition state theory

\begin{equation}
I_m = \pm k_m a_m \left|1 - \Omega_m^{\theta}\right|^{\eta},
\end{equation}
where $I_m$ is positive for dissolution and negative for precipitation, $k_m$ is the rate constant,
$a_m$ is the specific reactive surface area, $\Omega_m$ is termed the mineral saturation ratio,
expressed as

\begin{equation}
\Omega_m = \frac{1}{K_m} \prod_{j}(\gamma_j C_j)^{\nu_{jm}},
\end{equation}
where $K_m$ is the equilibrium constant for mineral $m$, $\gamma_j$ is the activity coefficient,
$C_j$ is the concentration of the $j^{\mathrm{th}}$ primary species and $\nu_{jm}$ is the
stoichiometric coefficient.

The rate constant $k_m$ is typically reported at a reference temperature (commonly
25$^{\circ}$C). Using an Arrhenius relation, the temperature dependence of $k_m$ is given as

\begin{equation}
k_m(T) = k_{m,0} \exp\left[\frac{E_a}{R} \left(\frac{1}{T_0} - \frac{1}{T}\right)\right],
\end{equation}
where $k_{m,0}$ is the rate constant at reference temperature $T_0$, $E_a$ is the activation
energy, $R$ is the gas constant.

The exponents $\theta$ and $\eta$ in the reaction rate equation are specific to each mineral
reaction, and should be measured experimentally. For simplicity, they are set to unity in this
module.

!syntax parameters /AuxKernels/KineticDisPreRateAux

!syntax inputs /AuxKernels/KineticDisPreRateAux

!syntax children /AuxKernels/KineticDisPreRateAux
