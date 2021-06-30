# Permeability

A porous material's insitu permeability tensor can take one of several forms. It can be constant,
spatially varying, or depend on porosity.

## Constant: [PorousFlowPermeabilityConst](/PorousFlowPermeabilityConst.md)

The simplest case where the permeability tensor is a constant tensor.

## Spatially-varying: [PorousFlowPermeabilityConstFromVar](/PorousFlowPermeabilityConstFromVar.md)

To implement a heterogeneous permeability distribution, PorousFlow also provides a
permeability formulation where the elements of the permeability tensor can be `AuxVariables`,
enabling permeability to vary spatially.

## Exponential: [PorousFlowPermeabilityExponential](/PorousFlowPermeabilityExponential.md)

A simple porosity-permeability model where
\begin{equation}
k_{ij} = k_{ij}^{0} e^{a\phi} \ ,
\end{equation}
where $\phi$ is the porosity, and $a$ is a user-defined constant.

## Kozeny-Carman: [PorousFlowPermeabilityKozenyCarman](/PorousFlowPermeabilityKozenyCarman.md)

Permeability is calculated from porosity using the Kozeny-Carman relationship [!citep](oelkers1996)
\begin{equation}
k_{ij} = k_{ij}^{0} \frac{\phi^{n}}{(1 - \phi)^{m}},
\end{equation}
where $n$ and $m$ are user-defined constants.

## Permeability with a solid phase

A solid phase (from chemical precipitation, for instance) can be included in the framework described
herein simply by setting its relative permeability to zero.  However, in this case, the absolute
permeability of the porous material should be
\begin{equation}
k = k^{\mathrm{without\ solid\ phase}}(1 - S_{\mathrm{s}})^{2} \ ,
\end{equation}
where $S_{\mathrm{s}}$ is the solid-phase saturation.


!bibtex bibliography
