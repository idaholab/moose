# MaskedBodyForce

!syntax description /Kernels/MaskedBodyForce

Implements a kernel for a source term/body force limited to a certain region by
a mask $m$ (material property). A function or a postprocessor can also be supplied
to multiply the source term by. Contributions added by this kernel have the form
\begin{equation}
-m C f P
\end{equation}
where $m$ is the mask (supplied as a material property), $C$ is the constant source
term/body force, $f$ is a function (optional), and $P$ is the value of a postprocessor
(optional).

!syntax parameters /Kernels/MaskedBodyForce

!syntax inputs /Kernels/MaskedBodyForce

!syntax children /Kernels/MaskedBodyForce

!bibtex bibliography
