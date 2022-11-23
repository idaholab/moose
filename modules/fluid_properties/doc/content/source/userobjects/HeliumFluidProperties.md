# HeliumFluidProperties

!syntax description /FluidProperties/HeliumFluidProperties

## Description

The `HeliumFluidProperties` class provides fluid properties for helium
[!cite](petersen).

The standard deviation $\sigma$ for density is approximately 0.03% at a pressure
of 0.1 MPa and 0.3% at 10 MPa, or

\begin{equation}
\sigma=0.03\sqrt{\frac{P}{10}}\ \%
\end{equation}

where $P$ is in MPa. The isobaric and isochoric specific heats are constant,
with an uncertainty at 273 K of 0.05% at 0.1 MPa
and 0.5% at 10 MPa. At higher temperature, the standard deviation is lower, and
approximately

\begin{equation}
\sigma=0.05\left(\frac{P}{10}\right)^{0.6-0.1T/T_o}\ \%
\end{equation}

where $T_o=273.16$K, $P$ is in MPa, and $T$ is in K.
The standard deviation of the dynamic viscosity is approximately 0.4% at
273 K and 2.7% at 1800 K, or approximately

\begin{equation}
\sigma = 0.0015T\ \%
\end{equation}

where $T$ is in K. The standard deviation of the thermal conductivity is
approximately 1% at 273 K and 6% at 1800K, or

\begin{equation}
\sigma=0.0035T\ \%
\end{equation}

where $T$ is in K. The speed of sound is calculated as [!cite](harlow)

\begin{equation}
c=\sqrt{\frac{-\left\lbrack\frac{P}{\rho^2}-\frac{C_v}{\left(\frac{\partial\rho}{\partial T}\right)_p}\right\rbrack}{C_v\left(\frac{\partial\rho}{\partial P}\right)_T\left(\frac{\partial T}{\partial \rho}\right)_P}}
\end{equation}

## Range of Validity

The HeliumFluidProperties UserObject is valid for:

- 273.15 K $\le$ $T$ $\le$ 1800 K and
- 0.1 MPa $\le$ $p$ $\le$ 10 MPa.

!syntax parameters /FluidProperties/HeliumFluidProperties

!syntax inputs /FluidProperties/HeliumFluidProperties

!syntax children /FluidProperties/HeliumFluidProperties

!bibtex bibliography
