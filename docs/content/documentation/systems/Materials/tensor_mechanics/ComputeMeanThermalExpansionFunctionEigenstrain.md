#ComputeMeanThermalExpansionEigenstrain

!description /Materials/ComputeMeanThermalExpansionFunctionEigenstrain
## Description

This model computes the eigenstrain tensor resulting from isotropic thermal expansion where the temperature-dependent thermal expansion is defined by a user-supplied function that describes the mean thermal expansion coefficient $\bar{\alpha}$ as a function of temperature, $T$. This function is defined relative to a reference temperature, $T_{ref}$, such that the total expansion at a given temperature relative to the refererence temperature is $\bar{\alpha}(T-T_{ref})$.  Following the notation of \cite{niffenegger2012proper}, $\bar{\alpha}_{(T_{ref},T)}$ is defined as:

$$
\bar{\alpha}_{(T_{ref},T)} = \frac{L_{(T)} - L_{(T_{ref})}}{L_{(T_{ref})}(T-T_{ref})}
$$

where $L_{T}$ is the length of a body at the current temperature, and $L_{T_{ref}}$ is the length of that body at the reference temperature.

It is important to emphasize that this reference temperature is tied to the definition of the thermal expansion function, and differs in general from the stress-free temperature for a specific simulation.  For the general case where the stress-free temperature, $T_{sf}$, differs from the reference temperature, the total thermal expansion eigenstrain is computed as:

$$
\boldsymbol{\epsilon}^{th} = (\bar{\alpha}_{(T_{ref},T)}(T-T_{ref}) - \bar{\alpha}_{(T_{ref},T_{sf})}(T_{sf}-T_{ref})) \boldsymbol{I}
$$

where $T$ is the current temperature and $\boldsymbol{I}$ is the identity matrix.  Note that this does not include a correction to account for the ratio of $L_{(T_{sf})}$ to $L_{(T_{ref})}$, because as discussed in \cite{niffenegger2012proper}, that ratio is very close to 1.

!parameters /Materials/ComputeMeanThermalExpansionFunctionEigenstrain

!inputfiles /Materials/ComputeMeanThermalExpansionFunctionEigenstrain

!childobjects /Materials/ComputeMeanThermalExpansionFunctionEigenstrain

## References
\bibliographystyle{unsrt}
\bibliography{docs/bib/tensor_mechanics.bib}
