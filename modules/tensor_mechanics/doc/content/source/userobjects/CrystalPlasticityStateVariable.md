# CrystalPlasticityStateVariable

## Description

`CrystalPlasticityStateVariable` represents a state variable, such as dislocation density in [crystal plasticity system](FiniteStrainUObasedCP.md). The initial values of this state variable are either provided in the input file `readInitialValueFromInline()` or read from the file `readInitialValueFromFile()`. The state variable evolves as $y_{n+1} = y_n + \dot{y}\cdot dt $ where $\dot{y}$ is the state variable evolution rate. The $\dot{y}$ can have multiple components, such as $\dot{y} = a_1 r_1 + a_2 r_2 + \cdots$ where "$a_1, a_2, \cdots$" are the `scale_factor` and $r_1, r_2, \cdots$ are the individual rate components.


!syntax parameters /UserObjects/CrystalPlasticityStateVariable

!syntax inputs /UserObjects/CrystalPlasticityStateVariable

!syntax children /UserObjects/CrystalPlasticityStateVariable

!bibtex bibliography
