# CrystalPlasticitySlipRateGSS

## Description

`CrystalPlasticitySlipRateGSS` derives from `CrystalPlasticitySlipRate` which calculates slip rate in the [crystal plasticity system](FiniteStrainUObasedCP.md). The slip system is read into this class. The Schmid tensor is generated in this class and the flow direction is calculated by `calcFlowDirection()`. The slip rate $\dot{g} = g(T, s\cdots, y\cdots)$ is a function of PK2 stress $$T$$, slip resistance $$s$$ and state variables $y$.


!syntax parameters /UserObjects/CrystalPlasticitySlipRateGSS

!syntax inputs /UserObjects/CrystalPlasticitySlipRateGSS

!syntax children /UserObjects/CrystalPlasticitySlipRateGSS

!bibtex bibliography
