# MFEMRWTE10IntegratedBC

!if! function=hasCapability('mfem')

## Overview

Adds the frequency space integrator corresponding to the value upon a cross-sectional boundary of the TE10 mode of an electromagnetic wave traveling through an infinite rectangular waveguide. The physical system in question is explained in more detail [here](https://phys.libretexts.org/Bookshelves/Electricity_and_Magnetism/Electromagnetics_II_(Ellingson%29/06%3A_Waveguides/6.09%3A_Rectangular_Waveguide-_TE_Modes).

If the parameter [!param](/BCs/MFEMRWTE10IntegratedBC/input_port) is set to `false`, corresponding to the output boundary of the waveguide, the following boundary integrator is applied:

!equation
i\mathrm{Im}[k]\mu^{-1}(\vec u, \vec v)_{\partial\Omega} \,\,\, \forall \vec v \in V

where $\vec u, \vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$, $k$ is a complex scalar coefficient defined as follows:

!equation
k = i\sqrt{(2\pi\nu)^2\mu\epsilon-\frac{\pi^2}{|\vec a_1|^2}}

where $\nu$ is the [!param](/BCs/MFEMRWTE10IntegratedBC/frequency) parameter, $\vec a_1$ is the port length vector, and $\mu$ and $\epsilon$ are the magnetic permeability and electric permittivity of the material in which the wave is propagating.

If the parameter [!param](/BCs/MFEMRWTE10IntegratedBC/input_port) is set to `true`, corresponding to the input boundary of the waveguide, the following boundary integrator is applied:

!equation
i\mathrm{Im}[k]\mu^{-1}(\vec u, \vec v)_{\partial\Omega} + 2\mathrm{Im}[k]\mu^{-1}(\vec n \times (\mathrm{Im}[\vec f]-i\mathrm{Re}[\vec f]), \vec v)_{\partial\Omega}  \,\,\, \forall \vec v \in V

where $\vec f$ is defined by

!equation
\vec f=\sqrt{\frac{2\omega\mu}{|\vec a_1||\vec a_2|\mathrm{Im}[k]}} \sin(\vec k_a \cdot \vec x)e^{-i\vec k_c \cdot \vec x}\hat e

with $\vec a_2$ being the port width vector, $\vec k_a=\frac{\pi}{|\vec a_1|} \frac{\vec a_2 \times \vec k_c}{|\vec a_2 \times \vec k_c|}$ and $\vec k_c=k \frac{\vec a_1 \times \vec a_2}{|\vec a_1 \times \vec a_2|}$. Lastly, $\hat e$ is a unit vector in the direction of $\vec k_c \times \vec k_a$.


## Example Input File Syntax

!listing test/tests/mfem/complex/complex_waveguide.i block=BCs

!syntax parameters /BCs/MFEMRWTE10IntegratedBC

!syntax inputs /BCs/MFEMRWTE10IntegratedBC

!syntax children /BCs/MFEMRWTE10IntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
