# InternalVolume

!syntax description /Postprocessors/InternalVolume

## Description

`InternalVolume` computes the volume of an enclosed space. The complete boundary of the enclosed
 space must be represented by the user specified side set. The volume of the domain is calculated as
 the integral over the surface of the domain, where the domain surface is specified by the boundary.

If the given side set points outward, `InternalVolume` will report a negative volume.

As an example, consider
\begin{equation}
\int_V div(F) dV = \int_{dS} F \cdot \hat{n} dS
\end{equation}
where $F$ is a field, $\hat{n}$ is the normal of the surface, $V$ is the volume of the domain, and $S$ is
the surface of the domain.

For simplicity in this example, we choose
\begin{equation}
F = \left[ x, 0, 0 \right]^T \quad \text{then} \quad div(F) = 1
\end{equation}

such that the integral becomes

\begin{equation}
\int_V dV = \int_{dS} x \cdot n[0] dS
\end{equation}

The volume of the domain is the integral over the surface of the domain of the x position of the
surface times the x-component of the normal of the surface.

## Example Input Syntax

!listing modules/combined/test/tests/internal_volume/hex8.i start=internalVolume end=dispZ

!syntax parameters /Postprocessors/InternalVolume

!syntax inputs /Postprocessors/InternalVolume

!syntax children /Postprocessors/InternalVolume
