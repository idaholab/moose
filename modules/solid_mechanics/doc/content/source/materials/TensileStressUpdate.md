# Tensile Stress Update

!syntax description /Materials/TensileStressUpdate

## Theory

Tensile, or Rankine, plasticity is designed to simulate a material
that fails when the maximum principal stress exceeds the material's tensile
strength.  Its yield function is therefore

\begin{equation}
  f =  \sigma_{III} - T \ ,
\end{equation}

where $\sigma_{III}$ is the maximum principal
stress (the largest eigenvalue of the stress tensor) and $T$ is the
tensile strength.

!alert note
Often the maximum principal is denoted by $\sigma_{I}$, but the notation used in this document is motivated by the C++ code.  The code uses the `dsymmetricEigenvalues` method of `RankTwoTensor` and this orders the eigenvalues from smallest to greatest.

One yield function is sufficient because of the definition
$\sigma_{I}\leq\sigma_{II}\leq\sigma_{III}$.  For instance, if during
the return-map process both $\sigma_{II}$ and $\sigma_{III}$ exceed
$T$ the corresponding admissible configuration is that both of them
are equal to $T$.  While one yield function is sufficient, it is
convenient to use three yield functions in total:

\begin{equation}
\begin{split}
  f_{0} & = \sigma_{III} - T \\
  f_{1} & = \sigma_{II} - T  \\
  f_{2} & = \sigma_{I} - T   \\
\end{split}
\end{equation}


The return-map algorithm first rotates $\sigma$ from the physical
frame to the
principal-stress frame (where $\sigma = \text{diag}(\sigma_{I}, \sigma_{II},
\sigma_{III})$).  The rotation matrices used are assumed not to change
during the return-map process: only $\sigma_{I}$, $\sigma_{II}$ and
$\sigma_{III}$ change.  Therefore, at the end of the
return-map process these rotation matrices may be used to find the
final stress in the physical frame.

The three yield functions are smoothed using the
method encoded in [MultiParameterPlasticityStressUpdate.md].

Additional considerations can be found in the [MultiParameterPlasticityStressUpdate.md]
or in the theory manual  (at [solid_mechanics/doc/theory/tensile.pdf](https://github.com/idaholab/moose/modules/solid_mechanics/doc/theory/tensile.pdf)).


!syntax parameters /Materials/TensileStressUpdate

!syntax inputs /Materials/TensileStressUpdate

!syntax children /Materials/TensileStressUpdate
