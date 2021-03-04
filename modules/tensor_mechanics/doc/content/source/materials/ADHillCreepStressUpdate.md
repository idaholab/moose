# Transversely Isotropic Creep Stress Update

!syntax description /Materials/ADHillCreepStressUpdate

## Description

This class computes a creep strain rate based on an equivalent deviatoric stress function that
is calculated as a function of Hill's function anisotropy parameters $F$, $G$, $H$, $L$, $M$, and $N$:
\begin{equation}
\tilde{q}(\sigma) = {[F(\sigma_{22} - \sigma_{33})^2 + G(\sigma_{33} - \sigma_{11})^2 + H(\sigma_{11} - \sigma_{22})^2
+ 2L\sigma_{23}^2 + 2M\sigma_{13}^2 + 2N\sigma_{12}^2]}^{1/2}
\end{equation}

The equivalent creep strain rate function may then be obtained as 
\begin{equation}
\dot{\epsilon} = A_{aniso} (\tilde{q})^{n}
\end{equation}

where $A_{aniso}$ is a creep coefficient and $n$ the creep exponent.

The effective creep strain increment is obtained within the framework of a generalized (Hill plasticity) radial return mapping, see
[ADGeneralizedRadialReturnStressUpdate](/ADGeneralizedRadialReturnStressUpdate.md). This class computes the
generalized radial return inelastic increment.

More details on the Hill-type creep material model may be found in [!cite](stewart2011anisotropic).

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/ad_anisotropic_creep/ad_aniso_creep_y_3d.i block=Materials/trial_creep_two

!syntax parameters /Materials/ADHillCreepStressUpdate

!syntax inputs /Materials/ADHillCreepStressUpdate

!syntax children /Materials/ADHillCreepStressUpdate

!bibtex bibliography
