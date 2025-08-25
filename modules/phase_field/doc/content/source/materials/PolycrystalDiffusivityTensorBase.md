# PolycrystalDiffusivityTensorBase

The PolycrystalDiffusivityTensorBase material builds a diffusivity tensor that distinguishes between
the bulk, grain boundaries (GBs), and free surfaces, thus allowing different diffusion
rates in those regions.

The diffusivity tensor $\mathbf{D}$ is given by
\begin{equation}
    \mathbf{D} = \mathbf{D}_B + \mathbf{D}_{GB} + \mathbf{D}_S
\end{equation}
where $\mathbf{D}_B$ is the bulk diffusion tensor, $\mathbf{D}_{GB}$ is the grain boundary diffusion tensor, and $\mathbf{D}_S$ is the surface diffusion tensor. The bulk diffusivity tensor is given by
\begin{equation}
    \mathbf{D_B} = D_B \mathbf{I}
\end{equation}
where $D_B$ is the scalar bulk diffusivity and $\mathbf{I}$ is the identity tensor. The bulk diffusion is calculated using the standard Arrhenius equation
$D_B = D_0 \exp\left(\frac{-E_m}{k_B T}\right)$.
The grain boundary and bulk diffusion magnitudes are approximated as multiples of the bulk
diffusion $D_{GB} = w_{GB}D_B$, $D_S = w_s D_B$, where $w_{GB}$ and $w_S$ are scaling factors. The grain boundary diffusivity tensor is given by
\begin{equation}
    \mathbf{D}_{GB} = D_{GB} \sum_{i=1}^n \sum_{j\ne i}^n \eta_i \eta_j \mathbf{T}_{GB}^{ij}
\end{equation}
where $\mathbf{T}_{GB}^{ij}$ is the normalized GB directional tensor for the GB between order parameters $\eta_i$ and $\eta_j$, and is given by
\begin{equation}
      \mathbf{T}_{GB}^{ij} = \mathbf{I} - \frac{\nabla \eta_i - \nabla \eta_j}{|\nabla \eta_i - \nabla \eta_j|} \otimes \frac{\nabla \eta_i - \nabla \eta_j}{|\nabla \eta_i - \nabla \eta_j|}
\end{equation}

The surface diffusivity tensor is given by
\begin{equation}
    \mathbf{D}_S = D_S \phi^2 (1-\phi)^2 \mathbf{T}_S
\end{equation}
where $\phi$ is the order parameter that distinguishes between solid and void phases and $\mathbf{T}_S$ is the normalized surface directional tensor, which is given by
\begin{equation}
    \mathbf{T}_S = \mathbf{I} - \frac{\nabla \phi}{|\nabla \phi|} \otimes \frac{\nabla \phi}{|\nabla \phi|}
\end{equation}

Once the diffusivity tensor is calculated, it can be used to calculate a mobility
tensor in your model. Further information on this formulation can be found in [!cite](Deng2012) and [!cite](Ahmed2013).

!syntax description /Materials/PolycrystalDiffusivityTensorBase

!syntax parameters /Materials/PolycrystalDiffusivityTensorBase

!syntax inputs /Materials/PolycrystalDiffusivityTensorBase

!syntax children /Materials/PolycrystalDiffusivityTensorBase

!bibtex bibliography
