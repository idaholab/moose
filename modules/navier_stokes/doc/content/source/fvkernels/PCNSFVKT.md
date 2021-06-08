# PCNSFVKT

!syntax description /FVKernels/PCNSFVKT

## Overview

This object implements the Kurganov-Tadmor [!citep](kurganov2000new) (KT) scheme for
computing inter-cell advective fluxes for the Euler equations. We will outline
some of the important equations below, drawing from
[!citep](greenshields2010implementation). The KT flux is a second-order
generalization of the Lax-Friedrichs flux. For a given face $f$ it can be written as

\begin{equation}
\label{eq:kt_flux}
\bm{F} = \alpha \phi_{f+}\bm{\Psi}_{f+} + \left(1 -
\alpha\right)\phi_{f-}\bm{\Psi}_{f-} + \omega_f\left(\bm{\Psi}_{f-} -
\bm{\Psi}_{f+}\right)
\end{equation}

where $\bm{\Psi}_{f\pm}$ represents the vector of advected quantities, and

\begin{equation}
\phi_{f\pm} = \epsilon_{f\pm}\bm{a}_{f\pm}\cdot\hat{n}
\end{equation}

where $\epsilon$ is the porosity, $\bm{a} = \lbrace u,v,w\rbrace$ where $u$,
$v$, and $w$ are the component particle velocities, and $\hat{n}$ is the normal
vector pointing from $+$ to $-$. This definition of $\phi$ is slightly different
from that in [!citep](greenshields2010implementation) in that it does not
contain the face area. This is because here we are essentially describing the
implementation in `PCNSFVKT` while area multiplication happens in the base class
`FVFluxKernel`. $\alpha$ is defined as

\begin{equation}
\alpha=
\begin{cases}
\frac{1}{2} &\text{for Kurganov-Tadmor}\\
\frac{\psi{_f+}}{\psi_{f+} + \psi_{f-}} &\text{for Kurganov, Noelle, Petrova}
\end{cases}
\end{equation}

where

\begin{equation}
\psi_{f+} = \text{max}\left(c_{f+} + \phi_{f+},\ c_{f-} + \phi_{f-},\ 0\right)\\
\psi_{f-} = \text{max}\left(c_{f+} - \phi_{f+},\ c_{f-} - \phi_{f-},\ 0\right)
\end{equation}

where $c$ is the locally computed speed of sound. The default method when
computing $\alpha$ and $\omega$ is Kurganov, Noelle, Petrova
[!citep](kurganov2001semidiscrete) (KNP) since it's reported [!citep](greenshields2010implementation) as being less
diffusive (enabling sharper front capturing) than the KT method of computing
$\alpha$ and $\omega$. $\omega$ is given by

\begin{equation}
\omega_f=
\begin{cases}
\alpha \text{max}\left(\psi_{f+},\ \psi_{f-}\right) &\text{for KT}\\
\alpha\left(1 - \alpha\right)\left(\psi_{f+} + \psi_{f-}\right) &\text{for KNP}
\end{cases}
\end{equation}

Interpolation of $\bm{\Psi}_{f\pm}$ is described in [Limiters/index.md].

!syntax parameters /FVKernels/PCNSFVKT

!syntax inputs /FVKernels/PCNSFVKT

!syntax children /FVKernels/PCNSFVKT

!bibtex bibliography
