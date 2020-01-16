# Horizon

!media conventional_deformation_gradient_horizon.jpg style=width:300px;padding-left:20px;float:right;
       caption=Configuration of a discrete material point in peridynamic theory

In peridynamics theory, material points separated by a finite distance interact directly with each other. This nonlocal interaction region for a material point within the bulk is called the $\textit{peridynamic horizon}$, in short, $\textit{horizon}$. Most commonly, the region is taken to be of spherical/circular shape and the radius of the horizon region is referred to as $\textit{horizon size}$, or simply the horizon.

Horizon is usually denoted using the symbol of $\mathcal{H}$, and its size as $\delta$. Material points can have different horizon size, horizon for material point $\mathbf{X}$ is denoted as $\mathcal{H}_{\mathbf{X}}$.

More discussion about $\textit{horizon}$ can be found in [!citep](Bobaru2012horizon).

# States

Let $\mathcal{L}_{m}$ denote the set of all tensors of order $\textit{m}$ (thus $\mathcal{L_{0}} = \mathbb{R}$).

A $\textit{state of order m}$ is a function $\underline{\mathbf{A}} \left\langle \cdot \right\rangle : \mathcal{H} \rightarrow \mathcal{L}_{m}$

Thus, the image of a vector $\mathbf{\xi} \in \mathcal{H}$ under the state $\underline{\mathbf{A}}$ is a tensor of order $\textit{m}$, written $\underline{\mathbf{A}}\left\langle \xi \right\rangle$. the set of all states of order $\textit{m}$ is denoted as $\mathcal{A}_{m}$. Angle brackets are used to indicate the vector on which a state operates.

A state of order 1 is called a $\textit{vector state}$. The set of all vector states is denoted as $\mathcal{V}$, thus, $\mathcal{V}=\mathcal{A}_{1}$. Vector states and other higher order states are usually written in uppercase bold font with an underscore, e.g., $\underline{\mathbf{A}}$.

A state of order 0 is called a $\textit{scalar state}$. The set of all scalar states is denoted as $\mathcal{S}$, thus $\mathcal{S}=\mathcal{A}_{0}$. Scalar states are usually written as lower case, non-bold font with an underscore, e.g., $\underline{a}$.

More information about $\textit{states}$ can be found in [!citep](Silling2007states).

The $\textit{relative position vector state}$ of two material points in reference configuration $\Omega_{r}$ is

\begin{equation}
  \underline{\mathbf{X}}\left\langle \xi \right\rangle = \boldsymbol{\xi} = \mathbf{X}^{\prime} - \mathbf{X}
\end{equation}
where $\mathbf{X}^{\prime}$ and $\mathbf{X}$ are the position vectors in reference configuration.

The $\textit{relative displacement vector state}$ of two material points is

\begin{equation}
  \underline{\mathbf{U}}\left[ \mathbf{X},t \right] \left\langle \xi \right\rangle = \boldsymbol{\eta} = \mathbf{u}\left( \mathbf{X}^{\prime},t \right) - \mathbf{u}\left( \mathbf{X},t\right)
\end{equation}
where $\mathbf{u}\left( \mathbf{X}^{\prime},t \right)$ and $\mathbf{u}\left( \mathbf{X},t\right)$ are the displacement vectors. The square bracket notation has the similar meaning to standard parentheses, indicating dependence on quantities, but is used for peridynamic states.

The $\textit{relative position vector state}$ or $\textit{deformation state}$ of two material points in the current configuration $\Omega_{c}$ is

\begin{equation}
  \underline{\mathbf{Y}}\left[ \mathbf{X},t \right] \left\langle \xi \right\rangle = \boldsymbol{\xi} + \boldsymbol{\eta} = \mathbf{x}^{\prime}\left( \mathbf{X}^{\prime},t \right) - \mathbf{x}\left( \mathbf{X},t\right)
\end{equation}
with
\begin{equation}
  \mathbf{x}\left( \mathbf{X},t \right) = \mathbf{X} + \mathbf{u}\left( \mathbf{X},t \right)
\end{equation}

\begin{equation}
  \mathbf{x}^{\prime} \left( \mathbf{X}^{\prime},t \right) = \mathbf{X}^{\prime} + \mathbf{u}\left( \mathbf{X}^{\prime},t \right)
\end{equation}
where $\mathbf{x}\left( \mathbf{X},t \right)$ and $\mathbf{x}^{\prime}\left( \mathbf{X}^{\prime},t\right)$ are the position vectors in the current configuration.
