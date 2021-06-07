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

We will now discuss computation of $\bm{\Psi}_{f\pm}$ ($\phi_{f\pm}$ is computed
in an identical way). To be clear about notation: the equations that follow will
have a lot of $\pm$ and $\mp$. When computing the "top" quantity (e.g. $+$ for
$\pm$) we select the "top" quantities throughout the equation, e.g. we select $+$
for $\pm$ and $-$ for $\mp$. Similarly, when computing "bottom" quantities we select the
"bottom" quantities throughout the equation. We will also have a series of "ors"
in the text. In general left of "or" will be for "top" quantities and right of
"or" will be for "bottom" quantities.

Interpolation of advected quantities proceeds as follows:

\begin{equation}
\bm{\Psi}_{f\pm} = \left(1 - g_{f\pm}\right)\bm{\Psi}_{\pm} +
g_{f\pm}\bm{\Psi}_{\mp}
\end{equation}

where $\bm{\Psi}_{\pm}$ denotes the $+$ or $-$ cell centroid value of the
interpolated quantity and

\begin{equation}
g_{f\pm} = \beta\left(r_{\pm}\right)\left(1 - w_{f\pm}\right)
\end{equation}

where $\beta\left(r_{\pm}\right)$ represents a flux limiter function and

\begin{equation}
w_{f\pm} = \vert \bm{d}_{f\mp}\vert/\vert \bm{d}_{\pm}\vert
\end{equation}

where $\bm{d}_{f-}$ is the norm of the distance from the face to the $-$ or $+$
cell centroid and $\bm{d}_{\pm}$ is the norm of the distance from the $+$ to $-$
or $-$ to $+$
cell centroids.

The flux limiter function takes different forms. For example the min-mod limiter
is expressed as $\beta_{\text{min-mod}}\left(r\right) = \text{max}\left(0,\
\text{min}\left(1,\ r\right)\right)$. $r_{\pm}$ is computed as follows

\begin{equation}
r_{\pm} = 2 \frac{\bm{d}_{\pm}\cdot\left(\nabla
\bm{\Psi}\right)_{\pm}}{\left(\nabla_d \bm{\Psi}\right)_{f\pm}} - 1
\end{equation}

where $\left(\nabla \bm{\Psi}\right)_{\pm}$ corresponds to the $+$ or $-$ cell
centroid gradient and $\left(\nabla_d \bm{\Psi}\right)_{f\pm} =
\bm{\Psi}_{\mp} - \bm{\Psi}_{\pm}$. A list and summary of the limiters available
in MOOSE can be found [here](Limiters/index.md).

!syntax parameters /FVKernels/PCNSFVKT

!syntax inputs /FVKernels/PCNSFVKT

!syntax children /FVKernels/PCNSFVKT

!bibtex bibliography
