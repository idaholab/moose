# CrystalPlasticityKalidindiBackstressUpdate

!syntax description /Materials/CrystalPlasticityKalidindiBackstressUpdate

`CrystalPlasticityKalidindiBackstressUpdate` is designed based on `CrystalPlasticityKalidindiUpdate` to account for the backstress term in the crystal plasticity constitutive model [!cite](kalidindi1992).

## Constitutive Model Definition
In the crystal plasticity finite element model, the main purpose of introducing the "backstress" term is to consider the nonlinear and history-dependent behavior of the crystal. This item can help simulate various complex plastic phenomena in crystals, especially under conditions such as non-uniform strain and cycle loading. Meanwhile, for a detailed introduction to the crystal plasticity model, please refer to [CrystalPlasticityKalidindiUpdate](/CrystalPlasticityKalidindiUpdate.md). Here the slip rate is given as a power law relationship considering backstress term: 
\begin{equation}
  \label{eqn:powerLawSlipRate}
  \dot{\gamma}^{\alpha}=\dot{\gamma}_o\left| \frac{\tau ^{\alpha}-\chi ^{\alpha}}{g^{\alpha}} \right|^{1/m}sign\left( \tau ^{\alpha}-\chi ^{\alpha} \right)
\end{equation}

The backstress term $\chi^{\alpha}$ of each slip system is solved with the Armstrong-Frederick type hardening-dynamic recovery law [!cite](Frederick2007):
\begin{equation}
  \label{eqn:backstressEvolution}
  \dot{\chi}^{\alpha}=c_{bs}\dot{\gamma}^{\alpha}-d_{bs}|\dot{\gamma}^{\alpha}|\chi ^{\alpha}
\end{equation}

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/stress_update_material_based/exception_backstress.i block=Materials/trial_xtalpl

!syntax parameters /Materials/CrystalPlasticityKalidindiBackstressUpdate

!syntax inputs /Materials/CrystalPlasticityKalidindiBackstressUpdate

!syntax children /Materials/CrystalPlasticityKalidindiBackstressUpdate

!bibtex bibliography