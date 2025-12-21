# TidalGravityAux

Computes a scalar gravity field `g(t,x)` for shallow-water models with an optional
simple global tide correction from the Sun and Moon, projected onto the local vertical.

!syntax description /AuxKernels/TidalGravityAux

The AuxKernel writes `g` for a MONOMIAL/CONSTANT AuxVariable so values are
element-wise constant and consistent on faces, matching how bathymetry is handled.

## Deriving the tide-generating acceleration (perturbation of $g$)

We work in a geocentric frame. Let

- $\mathbf r\in\mathbb R^3$ be the geocentric position of a point on (or near) Earth's surface,
- $\mathbf R\in\mathbb R^3$ be the geocentric position of the external body (Sun or Moon),
- $R=\|\mathbf R\|$, $\hat{\mathbf R}=\mathbf R/R$,
- $\mu=GM$ of the external body.

The goal is to obtain the +tidal acceleration+ $\mathbf a_{\text{tide}}$ that perturbs local gravity $\mathbf g_0 $ at the site, after removing the uniform acceleration of Earth's center toward the body.


### External potential and _free-fall_ correction

The Newtonian potential at $\mathbf r$ due to the external body is

\begin{equation}
\Phi(\mathbf r)=-\frac{\mu}{\|\mathbf R-\mathbf r\|}.
\end{equation}

If we adopt a frame comoving with Earth's center (i.e., in free fall toward the body), we must +remove the constant and linear terms+ of $\Phi$ expanded about $\mathbf r=\mathbf 0$. The remainder is the +tide-generating potential+ $U(\mathbf r)$.


### Quadrupole (second-order) expansion

For $\|\mathbf r\|\ll R$, expand with Legendre polynomials (or by Taylor series):
\begin{equation}
\frac{1}{\|\mathbf R-\mathbf r\|}
= \frac{1}{R}\sum_{n=0}^{\infty}\left(\frac{\|\mathbf r\|}{R}\right)^n P_n(\cos\psi),
\qquad
\cos\psi=\frac{\mathbf r\cdot\mathbf R}{\|\mathbf r\|\,R}.
\end{equation}

Keeping terms up to $n=2$ and then subtracting the $n=0$ (constant) and $n=1$ (uniform field) parts gives the leading retained piece, the +quadrupole+ term:
\begin{equation}
U(\mathbf r)\;=\;-\frac{\mu}{2R^3}\,\Big(\,\|\mathbf r\|^2-3(\hat{\mathbf R}\!\cdot\!\mathbf r)^2\Big).
\end{equation}

Equivalent tensor form: let $Q_{ij}=\delta_{ij}-3\,\hat R_i\hat R_j$. Then

\begin{equation}
U(\mathbf r) \;=\; -\frac{\mu}{2R^3}\, r_i Q_{ij} r_j.
\end{equation}


### Tidal acceleration (gradient of the potential)

By definition,

\begin{equation}
\mathbf a_{\text{tide}}(\mathbf r) \;=\; -\,\nabla U(\mathbf r).
\end{equation}

Taking the gradient of $U$ above,

\begin{equation}
\boxed{\;\mathbf a_{\text{tide}} \;=\; \frac{\mu}{R^3}\left(\mathbf r - 3\,\hat{\mathbf R}\,(\hat{\mathbf R}\!\cdot\!\mathbf r)\right)\;}
\end{equation}

This is the +differential+ gravitational field after removing the uniform acceleration of Earth's center. For multiple bodies (Sun, Moon), the total tide is the sum:

\begin{equation}
\boxed{\;\mathbf a_{\text{tide,\,total}} = \sum_b \frac{\mu_b}{\|\mathbf R_b\|^3}\left(\mathbf r - 3\,\hat{\mathbf R}_b\,(\hat{\mathbf R}_b\!\cdot\!\mathbf r)\right).}
\end{equation}


### Relation to _perturbation of_ $g$ and useful projections

A simple nominal (spherically symmetric) gravity at radius $r=\|\mathbf r\|$ is
\begin{equation}
\mathbf g_0 \approx -\frac{GM_E}{r^3}\,\mathbf r.
\end{equation}
A more accurate model can include Earth's ellipsoid, harmonics (e.g., EGM), and the centrifugal field of Earth's rotation. The +total gravity including tides+ is
\begin{equation}
\mathbf g = \mathbf g_0 + \mathbf a_{\text{tide}}.
\end{equation}

For vertical and horizontal components at the site, define the local radial unit vector
\begin{equation}
\hat{\mathbf n}=\frac{\mathbf r}{\|\mathbf r\|}.
\end{equation}
Then
\begin{equation}
\delta g_{\parallel} = \hat{\mathbf n}\cdot \mathbf a_{\text{tide}}, \qquad
\boldsymbol{\delta g}_{\perp}= \mathbf a_{\text{tide}} - \delta g_{\parallel}\,\hat{\mathbf n}.
\end{equation}

Here $\delta g_{\parallel}$ is the vertical (up/down) perturbation; $\boldsymbol{\delta g}_{\perp}$ is the horizontal tilt of the plumb line.


### Orders of magnitude

At Earth's surface, the vertical tide has peak amplitudes roughly
\begin{equation}
\delta g_{\parallel}^{\text{(Moon)}} \sim 1.1\times10^{-6} g, \qquad
\delta g_{\parallel}^{\text{(Sun)}} \sim 0.5\times10^{-6} g,
\end{equation}
where $g\approx 9.81\,\mathrm{m\,s^{-2}}$.


### Coordinate consistency and practical notes

- Use a +common frame+ for $\mathbf r$ and $\mathbf R_b$ at the +same instant+ (e.g., all in ECI, or all in ECEF after rotating).
- Units: meters for positions; $\mu_b=GM_b$ in $\mathrm{m^3\,s^{-2}}$.
- For mm/s$^2$ accuracy in geodesy, include: higher-order terms $O((r/R)^3)$, the centrifugal potential of Earth's rotation, Earth's figure/ellipsoidal gravity, and a consistent tide system (zero-tide, tide-free, mean-tide) per IERS conventions.


### Minimal vector form to implement

Given site vector $\mathbf r$ and body vectors $\mathbf R_b$:
\begin{equation}
\boxed{\;\mathbf a_{\text{tide}}=\sum_b\frac{\mu_b}{\|\mathbf R_b\|^3}\Big(\mathbf r-3\,\hat{\mathbf R}_b\,(\hat{\mathbf R}_b\!\cdot\!\mathbf r)\Big)\;, \quad
\hat{\mathbf R}_b=\frac{\mathbf R_b}{\|\mathbf R_b\|}.}
\end{equation}

With Earth-fixed radial unit $\hat{\mathbf n}=\mathbf r/\|\mathbf r\|$, the vertical and horizontal perturbations are
\begin{equation}
\delta g_{\parallel} = \hat{\mathbf n}\cdot \mathbf a_{\text{tide}}, \qquad
\boldsymbol{\delta g}_{\perp}= \mathbf a_{\text{tide}} - \delta g_{\parallel}\,\hat{\mathbf n}.
\end{equation}

!syntax parameters /AuxKernels/TidalGravityAux

!syntax inputs /AuxKernels/TidalGravityAux

!syntax children /AuxKernels/TidalGravityAux
