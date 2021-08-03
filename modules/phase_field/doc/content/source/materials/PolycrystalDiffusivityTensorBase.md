# PolycrystalDiffusivityTensorBase

The PolycrystalDiffusivityTensorBase material builds a diffusivity tensor that distinguishes between
the bulk, grain boundaries, and free surfaces, thus allowing different diffusion
rates in those regions.
The bulk diffusion is calculated using the standard Arrhenius Equation
$D_B = D_0 \exp\left(\frac{-E_m}{k_B T}\right)$.
The grain boundary and bulk diffusions are approximated as multiples of the bulk
diffusion $D_{GB} = w_{GB}D_B$, $D_S = w_s D_B$.
Once the diffusivity tensor is calculated, it can be used to calculate a mobility
tensor in your model.

!syntax description /Materials/PolycrystalDiffusivityTensorBase

!syntax parameters /Materials/PolycrystalDiffusivityTensorBase

!syntax inputs /Materials/PolycrystalDiffusivityTensorBase

!syntax children /Materials/PolycrystalDiffusivityTensorBase

!bibtex bibliography
