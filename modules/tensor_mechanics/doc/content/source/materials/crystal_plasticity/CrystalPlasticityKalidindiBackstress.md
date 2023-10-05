# CrystalPlasticityKalidindiBackstress

!syntax description /Materials/CrystalPlasticityKalidindiBackstress

`CrystalPlasticityKalidindiBackstress` is designed based on `CrystalPlasticityKalidindiUpdate` to account for the backstress term in the crystal plasticity constitutive model [!cite](kalidindi1992).

## Constitutive Model Definition
<!-- TODO A more detailed description of the physical background is needed. -->

Here the slip rate is given as a power law relationship considering backstress term:
\begin{equation}
  \label{eqn:powerLawSlipRate}
  \dot{\gamma}^{\alpha}=\dot{\gamma}_o\left| \frac{\tau ^{\alpha}-\chi ^{\alpha}}{g^{\alpha}} \right|^{1/m}sign\left( \tau ^{\alpha}-\chi ^{\alpha} \right)
\end{equation}
The backstress term $\chi^{\alpha}$ of each slip system is solved with the Armstrong-Frederick type hardening-dynamic recovery law [!cite](Frederick2007):
as a function of the slip increment
\begin{equation}
  \label{eqn:backstressEvolution}
  \dot{\chi}^{\alpha}=c_b\dot{\gamma}^{\alpha}-d_b|\dot{\gamma}^{\alpha}|\chi ^{\alpha}
\end{equation}

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/stress_update_material_based/exception_backstress.i block=Materials/trial_xtalpl

!syntax parameters /Materials/CrystalPlasticityKalidindiBackstress

!syntax inputs /Materials/CrystalPlasticityKalidindiBackstress

!syntax children /Materials/CrystalPlasticityKalidindiBackstress

!bibtex bibliography