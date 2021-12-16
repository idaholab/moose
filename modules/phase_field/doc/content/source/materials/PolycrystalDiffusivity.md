# PolycrystalDiffusivity

!syntax description /Materials/PolycrystalDiffusivity

The `PolycrystalDiffusivity` material builds a diffusivity function that distinguishes between
the solid and void phases, grain boundaries, and free surfaces, thus allowing different diffusion
rates in those regions.
The final diffusion function is expressed as:

$D = D_m \, h_m \, I_m + D_v \, h_v \, I_v + 30 \, D_s \, c^2 \, (1-c)^2 \, I_s + 9 \, D_{GB}\, I_{GB} \sum_i \sum_{j \neq i} \eta_{i}^2 \eta_{j}^2$.

Here, $D_m$ is the bulk diffusion coefficient in the matrix/solid phase, $D_v$ is the diffusion coefficient within the void/pore/bubble phase, $D_s$ is the coefficient for the surface diffusion,
and $D_{GB}$ is the coefficient of diffusion along the grain boundaries. The prefactors for the diffusion along various interfaces are chosen such that, $\int_0^1{ D_i f(\eta_i) \, d\eta_i} \approx D_i$, where $D_i$ denotes the diffusivity along the interface (i.e., $D_s$ or $D_{GB}$). $I_m$, $I_v$, $I_s$, $I_{GB}$ are the indices to activate different diffusion mechanisms. When the diffusion values in different regions are unknown, replace those with weighted bulk diffusion coefficients.


!syntax parameters /Materials/PolycrystalDiffusivity

!syntax inputs /Materials/PolycrystalDiffusivity

!syntax children /Materials/PolycrystalDiffusivity

!bibtex bibliography
