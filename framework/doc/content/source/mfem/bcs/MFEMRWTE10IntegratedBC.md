# MFEMRWTE10IntegratedBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMRWTE10IntegratedBC

## Overview

Adds the frequency space integrator corresponding to the value upon a cross-sectional boundary of the TE10 mode of an electromagnetic wave traveling through an infinite rectangular waveguide. The physical system in question is explained in more detail [here](https://phys.libretexts.org/Bookshelves/Electricity_and_Magnetism/Electromagnetics_II_(Ellingson)/06%3A_Waveguides/6.09%3A_Rectangular_Waveguide-_TE_Modes). 

If the parameter `input` is set to `false`, corresponding to the output boundary of the waveguide, the following boundary integrator is applied:

!equation
iIm[k]\mu_0^{-1}(\vec u, \vec v)_{\partial\Omega} \,\,\, \forall v \in V

where $\vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$, $k$ is a complex scalar coefficient defined as follows:

!equation
k = i\sqrt{(2\pi\nu)^2\mu_0\epsilon_0-\frac{\pi}{|\vec a_1|}}

where $\nu$ is the `freq` parameter, $\vec a_1$ is the port length vector, and $\mu_0$ and $\epsilon_0$ are the magnetic permeability and electric permittivity of the material in which the wave is propagating. By default, these are set to be the permeability and permittivity of vacuum.

If the parameter `input` is set to `true`, corresponding to the input boundary of the waveguide, the following boundary integrator is applied:

!equation
iIm[k]\mu_0^{-1}(\vec u, \vec v)_{\partial\Omega} + 2Im[k]\mu_0^{-1}(\vec n \times (Im[\vec f]-iRe[\vec f]), \vec v)_{\partial\Omega}  \,\,\, \forall v \in V

where $\vec f$ is defined by 

!equation
\vec f=\sqrt{\frac{2\omega\mu_0}{|\vec a_1||\vec a_2|Im[k]}} \sin(\vec k_a \cdot \vec x)e^{-i\vec k_c \cdot x}\hat e

with $\vec a_2$ being the port width vector, $\vec k_a=\vec a_2 \times \vec k_c$ and $\vec k_c=\vec a_1 \times \vec a_2$. Lastly, $\hat e$ is a unit vector in the direction of $\vec k_c \times \vec k_a$.


## Example Input File Syntax

!listing test/tests/mfem/kernels/complex_waveguide.i block=BCs

!syntax parameters /BCs/MFEMRWTE10IntegratedBC

!syntax inputs /BCs/MFEMRWTE10IntegratedBC

!syntax children /BCs/MFEMRWTE10IntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
