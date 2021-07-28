# GrandPotentialTensorMaterial

Mobility calculations for the grand potential model using a tensor vacancy mobility.
GrandPotentialTensorMaterial calculates the vacancy mobility tensor $\mathbf{M}=\chi \mathbf{D}$
and the solid and void phase order parameter mobilities $L=\frac43 \frac{M}l$.

This material uses the diffusion tensor defined in ```DiffMtrxBase``` for the vacancy
diffusivity.

!syntax parameters /Materials/GrandPotentialTensorMaterial

!syntax inputs /Materials/GrandPotentialTensorMaterial

!syntax children /Materials/GrandPotentialTensorMaterial

!bibtex bibliography
