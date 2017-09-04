# ComputeCrackPropagationHeatEnergy

Calculate crack propagation heat source:
$$
- \frac{d\Psi}{dc} \cdot \frac{dc}{dt}
$$
$$
\Psi = (1 - c)^2 \cdot G_{0,+} + G_{0,-}
$$
$$
- \frac{d\Psi}{dc} \cdot \frac{dc}{dt} = 2 \cdot (1 - c) \cdot G_{0,+} \cdot \frac{dc}{dt}
$$
\cite{miehe2015phasefield}
\cite{chakraborty2016intergranularfracture}

!syntax description /Materials/ComputeCrackPropagationHeatEnergy

!syntax parameters /Materials/ComputeCrackPropagationHeatEnergy

!syntax inputs /Materials/ComputeCrackPropagationHeatEnergy

!syntax children /Materials/ComputeCrackPropagationHeatEnergy
