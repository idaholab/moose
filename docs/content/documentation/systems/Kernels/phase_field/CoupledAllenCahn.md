# CoupledAllenCahn
!description /Kernels/CoupledAllenCahn

Implements the term

$$
L(\eta,a,b,\dots)\frac{\delta F}{\delta\eta} = L(\eta,a,b,\dots)\frac{\partial f(\eta,a,b,\dots)}{\partial\eta}
$$

$F$ is the free energy functional of the system that is defined as $F=\int f(\eta) d\Omega$.

$\eta$ (`v`) is a coupled non-conserved order parameter, $L$ (`mob_name`) its associated mobility,
$f$ (`f_name`) is a free energy density provided by a [function material](../../introduction/FunctionMaterials), and
$a,b,\dots$ (`args`) are additional variable dependencied of the mobility and free energy density.

!parameters /Kernels/CoupledAllenCahn

!inputfiles /Kernels/CoupledAllenCahn

!childobjects /Kernels/CoupledAllenCahn
