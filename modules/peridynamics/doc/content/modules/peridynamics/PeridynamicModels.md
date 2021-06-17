# Peridynamic Mechanics Models

!media peridynamic_models.png style=width:1000px;padding-left:20px;float:top;
       caption=Schematics of bond-based (a), ordinary (b) and non-ordinary (c) state based peridynamic material response

The first peridynamic model, termed the bond-based peridynamic model (BPD), was proposed by Silling in the year of 2000 [!citep](Silling2000bond). In BPD, material points interact in a pair-wise fashion with neighboring material points that fall within their horizon. The interaction between two material points depends only on their own deformations.

Later, Silling et al. [!citep](Silling2007states) generalized BPD model in what is termed state-based peridynamic models (SPD) by introducing the concept of states. In SPD, the force state between two material points depends not only on their own deformation states, but also on the deformation states of other material points within their horizons. Depending on the direction of force state between a material point pair, SPD models can be classified into ordinary state-based peridynamic (OSPD) and non-ordinary state-based peridynamic (NOSPD) models.

SPD models overcome several issues within BPD model, such as only permitting a fixed Poisson's ratio of 0.25, inconsistency in modeling plastic deformation for metals, and requiring a complete recast of standard continuum material models in terms of pairwise force function to permit their use within peridynamics. A review of BPD and SPD and their applications can be found in [!citep](Bobaru2016handbook).

In peridynamics theory, the Equations of Motion (EOM) for a material point $\mathbf{X}$ in the reference configuration at time $t$ is given by

\begin{equation}
  \rho\left(\mathbf{X}\right) \ddot{\mathbf{u}}\left( \mathbf{X},t \right) = \int_{\mathcal{H}_{\mathbf{X}}}\mathbf{f}\left(\mathbf{X},\mathbf{X}^{\prime},t \right)dV_{X^{\prime}} + \mathbf{b}\left( \mathbf{X},t \right), \hspace{20 pt} \forall\left(\mathbf{X},t\right) \in \Omega_{r} \times \left(0, \tau \right)
\end{equation}
where $\rho\left(\mathbf{X} \right)$ is the mass density, $\mathbf{f}\left(\mathbf{X}, \mathbf{X}^{\prime}, t\right)$ is the force density function, and $\mathbf{b}\left(\mathbf{X},t\right)$ is external force density vector.

Depending on different peridynamics models, the force density function has different formulation. Various formulations for force density functions are outlined below.

## Bond-based models

\begin{equation}
  \mathbf{f}\left(\mathbf{X}, \mathbf{X}^{\prime}, t\right) = cs\mathbf{M}
\end{equation}
with $\textit{micro-modulus}$ $c$ has values of

for $\textbf{regular uniform}$ spatial discretization

\begin{equation}
  c = \left\{\begin{matrix}
    12K^{\prime} / \pi h_2 \delta^{3} & \text{two dimensional} \\
    18K / \pi \delta^{4} & \text{three dimensional}
  \end{matrix}\right.
\end{equation}

for $\textbf{irregular non-uniform}$ spatial discretization

\begin{equation}
  c = \left\{ \begin{matrix}
    4K^{\prime} / \left| \boldsymbol{\xi} \right| \cdot c_v & \text{two dimensional} \\
    9K / \left| \boldsymbol{\xi} \right| \cdot c_v & \text{three dimensional}
    \end{matrix} \right.
\end{equation}
with
\begin{equation}
  c_v = \frac{\int_{\mathcal{H}_{\mathbf{X}}}dV +  \int_{\mathcal{H}_{\mathbf{X}^{\prime}}} dV} {\int_{\mathcal{H}_{\mathbf{X}}} dV \cdot \int_{\mathcal{H}_{\mathbf{X}^{\prime}}} dV}
\end{equation}
where $K$ is the bulk modulus and $K^{\prime}$ is the two-dimensional bulk modulus, $E$ is the Young's modulus, $h_2$ is the plane thickness and $h_1$ is the cross-sectional area.
\begin{equation}
  K^{\prime} = \left\{ \begin{matrix}
    E/2\left(1-v\right) & \text{plane stress} \\
    E/2(1-v-2v^2) & \text{plane strain}
  \end{matrix} \right.
\end{equation}
where $v$ is the Poisson's Ratio.

$s$ being the $\textit{bond stretch}$ and defined as the change in bond length divided by initial bond length

\begin{equation}
  s = \frac{e}{\left| \boldsymbol{\xi} \right|} = \frac{\left| \mathbf{x}^{\prime}\left(\mathbf{X}^{\prime}, t \right) - \mathbf{x} \left( \mathbf{X},t\right)\right| - \left| \mathbf{X}^{\prime} - \mathbf{X} \right|}{\left| \mathbf{X}^{\prime} - \mathbf{X} \right|}  = \frac{\left| \boldsymbol{\xi} + \boldsymbol{\eta} \right| - \left| \boldsymbol{\xi} \right|}{\left| \boldsymbol{\xi} \right|}
\end{equation}

$\mathbf{M}$ being the unit vector in the direction of the deformed bond from $\mathbf{X}$ to $\mathbf{X}^{\prime}$

\begin{equation}
  \mathbf{M} = \frac{\mathbf{x}^{\prime} \left( \mathbf{X}^{\prime}, t \right) - \mathbf{x} \left( \mathbf{X}, t \right)}{\left| \mathbf{x}^{\prime} \left( \mathbf{X}^{\prime}, t \right) - \mathbf{x} \left( \mathbf{X}, t \right) \right|} = \frac{\boldsymbol{\xi} + \boldsymbol{\eta}}{\left| \boldsymbol{\xi} + \boldsymbol{\eta} \right|}
\end{equation}

## Ordinary state-based models

\begin{equation}
  \mathbf{f} \left( \mathbf{X}, \mathbf{X}^{\prime}, t \right) = \underline{\mathbf{t}} \left( \mathbf{X}, t \right) - \underline{\mathbf{t}} \left(\mathbf{X}^{\prime}, t \right)
\end{equation}
with force density vector $\underline{\mathbf{t}}$ as

\begin{equation}
  \underline{\mathbf{t}} \left( \mathbf{X}, t \right) = 2 \delta \left( d_{\mathbf{X}} \mathbf{M} \cdot \frac{\boldsymbol{\xi}}{\left| \boldsymbol{\xi} \right|} a \theta_{\mathbf{X}} + b s \right) \mathbf{M}
\end{equation}

\begin{equation}
  \underline{\mathbf{t}} \left( \mathbf{X}^{\prime}, t \right) = -2 \delta \left( d_{\mathbf{X}^{\prime}} \mathbf{M} \cdot \frac{\boldsymbol{\xi}}{\left| \boldsymbol{\xi} \right|} a \theta_{\mathbf{X}^{\prime}} + b s \right) \mathbf{M}
\end{equation}
where $s$ is the bond stretch, $\theta$ is the dilatation at a material point which can be calculated as

\begin{equation}
  \theta = d \delta \int_{\mathcal{H}} s \frac{\boldsymbol{\xi}}{\left| \boldsymbol{\xi} \right|} \cdot \mathbf{M} dV
\end{equation}

and

\begin{equation}
  a = \left\{\begin{matrix}
    \left( K^{\prime} - 2 \mu \right) / 2 & \text{two dimensional} \\
    \left( 3 K - 5 \mu \right) / 2 & \text{three dimensional}
  \end{matrix}\right.
\end{equation}

for $\textbf{regular uniform}$ spatial discretization

\begin{equation}
  d = \left\{\begin{matrix}
    2 / \left( \pi h_2 \delta^3 \right) & \text{two dimensional} \\
    9 / \left( 4 \pi \delta^4 \right) & \text{three dimensional}
  \end{matrix}\right.
\end{equation}

\begin{equation}
  b = \left\{\begin{matrix}
    6 \mu / \left( \pi h_2 \delta^4 \right) & \text{two dimensional} \\
    15 \mu / \left( 2 \pi \delta^5 \right) & \text{three dimensional}
  \end{matrix}\right.
\end{equation}

for $\textbf{irregular non-uniform}$ spatial discretization

\begin{equation}
  d = \left\{\begin{matrix}
    2 \bigg/ \left(\delta \int_{\mathcal{H}} dA \right) & \text{two dimensional} \\
    3 \bigg/ \left(\delta \int_{\mathcal{H}} dV \right) & \text{three dimensional}
  \end{matrix}\right.
\end{equation}

\begin{equation}
  \delta_{\mathbf{X}} b_{\mathbf{X}} + \delta_{\mathbf{X}^{\prime}} b_{\mathbf{X}^{\prime}} = \left\{ \begin{matrix}
    2 \left( K^{\prime} / 2 - a \right) / \left| \boldsymbol{\xi} \right| & \text{two dimensional} \\
    3 \left( K / 2 - a \right) / \left| \boldsymbol{\xi} \right| & \text{three dimensional}
  \end{matrix} \right\} \cdot \left( \delta_{\mathbf{X}} d_{\mathbf{X}} + \delta_{\mathbf{X}^{\prime}} d_{\mathbf{X}^{\prime}} \right)
\end{equation}
with $\mu$ is the shear modulus.

Reference for case of regular uniform spatial discretization can be found at [!citep](Madenci2014book) and [!citep](VanLe2018). For case of irregular non-uniform spatial discretization can be found at [!citep](Hu2018irregular).

## Non-ordinary state-based models

The general expression for force density function can be written as:
\begin{equation}
  \mathbf{f} \left( \mathbf{X}, \mathbf{X}^{\prime}, t \right) = \underline{\mathbf{T}} \left[ \mathbf{X}, t\right] \left\langle \mathbf{X}^{\prime} - \mathbf{X} \right\rangle - \underline{\mathbf{T}}\left[ \mathbf{X}^{\prime}, t \right] \left\langle \mathbf{X} - \mathbf{X}^{\prime} \right\rangle
\end{equation}
where $\underline{\mathbf{T}} \left[ \mathbf{X}, t \right] \left\langle \mathbf{X}^{\prime} - \mathbf{X} \right\rangle$, in short $\underline{\mathbf{T}} \left\langle \boldsymbol{\xi} \right\rangle$, is the force density state exerted on material point $\mathbf{X}$ from $\mathbf{X}^{\prime}$, while $\underline{\mathbf{T}}\left[\mathbf{X}^{\prime}, t \right] \left\langle \mathbf{X} - \mathbf{X}^{\prime} \right\rangle$, in short $\underline{\mathbf{T}} \left\langle  - \boldsymbol{\xi} \right\rangle$, is the force density state exerted on material point $\mathbf{X}^{\prime}$ from $\mathbf{X}$.

### Conventional material correspondence model

\begin{equation}
  \underline{\mathbf{T}} \left\langle \boldsymbol{\xi} \right\rangle = \underline{\omega} \left\langle \boldsymbol{\xi} \right\rangle \mathbf{P}_{\mathbf{X}} \mathbf{K}_{\mathbf{X}}^{-1} \boldsymbol{\xi}
\end{equation}

\begin{equation}
  \underline{\mathbf{T}} \left\langle - \boldsymbol{\xi} \right\rangle = - \underline{\omega} \left\langle \boldsymbol{\xi} \right\rangle \mathbf{P}_{\mathbf{X}^{\prime}} \mathbf{K}_{\mathbf{X}^{\prime}}^{-1} \boldsymbol{\xi}
\end{equation}
where $\mathbf{P}$ is the first Piola-Kirchhoff stress tensor and $\mathbf{K}$ is the shape tensor. Definition of shape tensor can be found on [Deformation Gradients](peridynamics/DeformationGradients.md) page.

### Bond-horizon stabilized material correspondence models

Two formulations of bond-horizon stabilized material correspondence model are available. In Form I formulation [!citep](Chen2018bond1, Chen2019bond2), although the calculation of bond-associated deformation gradient (see [Deformation Gradients](peridynamics/DeformationGradients.md) page), hence bond-associated force state, uses deformation states of other material points within a material point's horizon, this formulated bond-associated deformation gradient does not contribute to the force state of other bonds connecting the reference material point with these material points. To compensate this nonlocal effect, a volume fraction factor is used in Form I formulation to approximate the correct force states. However, this greatly reduces the nonlocality of the formulation, hence is computationally less expensive. In Form II formulation, this nonlocal effect is explicitly accounted, i.e., the force state of a bond is the sum of contributions from all bond-associated deformation gradient. No volume fraction factor is used. However, a weight function based on a volume fraction is used to distribute the energy associated with a material point to different bond-associated horizons.

In Form I formulation, the force states have the following expressions:

\begin{equation}
  \underline{\mathbf{T}}\left[\mathbf{X},t\right]\left\langle \mathbf{X}^{\prime}-\mathbf{X} \right\rangle = \frac{\int_{\mathcal{H}_{\mathbf{X}} \cap h_{\boldsymbol{\xi}}}1dV_{\mathbf{X}''}}{\int_{\mathcal{H}_{\mathbf{X}}}1 dV_{\mathbf{X}''}} \underline{\omega}\left\langle \boldsymbol{\xi} \right\rangle \mathbf{P}_{\boldsymbol{\xi}} \mathbf{K}_{\boldsymbol{\xi}}^{-1} \boldsymbol{\xi}
\end{equation}

\begin{equation}
  \underline{\mathbf{T}}\left[\mathbf{X}^{\prime},t\right]\left\langle \mathbf{X}-\mathbf{X}^{\prime} \right\rangle = \frac{\int_{\mathcal{H}_{\mathbf{X}^{\prime}} \cap h_{-\boldsymbol{\xi}}}1dV_{\mathbf{X}''}}{\int_{\mathcal{H}_{\mathbf{X}^{\prime}}}1 dV_{\mathbf{X}''}} \underline{\omega}\left\langle -\boldsymbol{\xi} \right\rangle \mathbf{P}_{-\boldsymbol{\xi}} \mathbf{K}_{-\boldsymbol{\xi}}^{-1} (-\boldsymbol{\xi})
\end{equation}

In Form II formulation, the force states have the following expressions:

\begin{equation}
  \underline{\mathbf{T}}\left[\mathbf{X},t\right]\left\langle \mathbf{X}^{\prime}-\mathbf{X} \right\rangle =
  \sum_{n=1}^{N_{\mathbf{\xi}}} \frac{\int_{\mathcal{H}_{\mathbf{X}} \cap h_{\mathbf{X}_{n}^{\prime}}}1dV_{\mathbf{X}''}}{\sum_{m=1}^{N_{\mathbf{X}}} \int_{\mathcal{H}_{\mathbf{X}} \cap h_{\mathbf{X}_{m}^{\prime}}}1dV_{\mathbf{X}''}} \underline{\omega}\left\langle \boldsymbol{\xi} \right\rangle \mathbf{P}_{\boldsymbol{\xi},n} \mathbf{K}_{\boldsymbol{\xi},n}^{-1} \boldsymbol{\xi}
\end{equation}

\begin{equation}
  \underline{\mathbf{T}}\left[\mathbf{X}^{\prime},t\right]\left\langle \mathbf{X}-\mathbf{X}^{\prime} \right\rangle =
  \sum_{n=1}^{N_{-\mathbf{\xi}}} \frac{\int_{\mathcal{H}_{\mathbf{X}^{\prime}} \cap h_{\mathbf{X}_{n}}}1dV_{\mathbf{X}''}}{\sum_{m=1}^{N_{\mathbf{X}^{\prime}}} \int_{\mathcal{H}_{\mathbf{X}^{\prime}} \cap h_{\mathbf{X}_{m}}}1dV_{\mathbf{X}''}} \underline{\omega}\left\langle -\boldsymbol{\xi} \right\rangle \mathbf{P}_{-\boldsymbol{\xi},n} \mathbf{K}_{-\boldsymbol{\xi},n}^{-1} (-\boldsymbol{\xi})
\end{equation}
where $N_{\mathbf{X}$ and $N_{\mathbf{\xi}}$ are the total number of material points in the material point horizon and the intersect of the material point horizon and a bond-associated horizon.

It should be noted that the First Piola-Kirchhoff stress tensor $\mathbf{P}$ and shape tensor $\mathbf{K}$ are all bond-associated quatities.

# Peridynamic Heat Conduction Models

The peridynamic heat conduction equation is

\begin{equation}
  \rho\left(\mathbf{X} \right) C \dot{T} \left( \mathbf{X}, t \right) = \int_{\mathcal{H}_{\mathbf{X}}} f \left( \mathbf{X},\mathbf{X}^{\prime}, t \right)dV_{X^{\prime}} + q \left( \mathbf{X},t \right), \hspace{20 pt} \forall \left( \mathbf{X}, t \right) \in \Omega_{r} \times \left( 0, \tau \right)
\end{equation}
where $C$ is the specific heat capacity, $f$ is the response function, and $q$ is specific heat source or sink.

## Bond-based models

\begin{equation}
  f \left(\mathbf{X}, \mathbf{X}^{\prime}, t \right) = k \frac{T \left( \mathbf{X}^{\prime}, t \right) - T \left( \mathbf{X}, t \right)}{\left| \mathbf{\xi} \right|}
\end{equation}
with $\textit{micro-conductivity}$ $k$ has values of

for $\textbf{regular uniform}$ spatial discretization

\begin{equation}
  k = \left\{\begin{matrix}
    6 \lambda / \left( \pi h_2 \delta^{3} \right) & \text{two dimensional} \\
    6 \lambda / \left( \pi \delta^{4} \right) & \text{three dimensional}
  \end{matrix}\right.
\end{equation}

for $\textbf{irregular non-uniform}$ spatial discretization

\begin{equation}
  k = \left\{ \begin{matrix}
    2 \lambda / \left| \boldsymbol{\xi} \right| \cdot c_v & \text{two dimensional} \\
    3 \lambda / \left| \boldsymbol{\xi} \right| \cdot c_v & \text{three dimensional}
  \end{matrix} \right.
\end{equation}
where $\lambda$ is the material thermal conductivity, $c_v$ is the volume coefficient which has the same expression as in bond-based mechanics model.

# Coupled Peridynamic Thermo-Mechanical Models

For coupled peridynamic thermo-mechanical modeling, the temperature from heat conduction model will contribute to the mechanical stretch and strain calculation in the mechanics model. And the kinematics can be written as

For bond stretch in bond-based and ordinary state-based mechanics models,

\begin{equation}
 s_{elastic} = s - \alpha \frac{\Delta T \left( \mathbf{X}, t \right) + \Delta T \left( \mathbf{X}^{\prime}, t \right)}{2}
\end{equation}

and the dilatation in ordinary state-based mechanics models

\begin{equation}
  \theta = d \delta \int_{\mathcal{H}} s_{elastic} \frac{\boldsymbol{\xi}}{\left| \boldsymbol{\xi} \right|} \cdot \mathbf{M} dV + 3 \alpha \Delta T
\end{equation}

Besides, the force density vector $\underline{\mathbf{t}}$ needs to be updated considering the thermal effect as

\begin{equation}
  \underline{\mathbf{t}} \left( \mathbf{X}, t \right) = 2 \delta \left( d_{\mathbf{X}} \mathbf{M} \cdot \frac{\boldsymbol{\xi}}{\left| \boldsymbol{\xi} \right|} \left( a \theta_{\mathbf{X}} - \frac{1}{2} a_{2} \Delta T_{\mathbf{X}} \right) + b s \right) \mathbf{M}
\end{equation}
where $a_{2} = 6 \alpha a$ for three-dimensional analysis and $a_{2} = 4 \alpha a$ for two-dimensional analysis.

For non-ordinary state-based mechanics models, the mechanical strain tensor is the subtraction of thermal eigen-strain from the total strain.

\begin{equation}
 \mathbf{\epsilon}_{mechanical} = \mathbf{\epsilon}_{total} - \alpha \Delta T \mathbf{I}
\end{equation}

In return, the bond status determined from the mechanics models will affect the response between material points in peridynamic heat conduction models.
