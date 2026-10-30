# ThermalCopperProperties

!syntax description /SolidProperties/ThermalCopperProperties

## Description

This userobject provides thermal properties for oxygen-free high-conductivity (OFHC) copper
as a function of temperature using correlations from the NIST Cryogenic Materials Database [!cite](nist_copper).
OFHC copper is used as a stabilizer/conductor in superconducting magnets such as those in the ITER central solenoid.

!include solid_properties_units.md

### Thermal Conductivity

Thermal conductivity depends on the residual resistivity ratio (RRR), which characterizes the purity
and defect structure of the copper. Higher RRR values correspond to higher thermal conductivity,
especially at cryogenic temperatures. Five RRR values are available: 50, 100, 150, 300, and 500,
with RRR = 100 as the default.

The NIST correlation for thermal conductivity is given as:

\begin{equation}
\log_{10}(k) = \frac{a + c\sqrt{T} + eT + gT^{1.5} + iT^2}{1 + b\sqrt{T} + dT + fT^{1.5} + hT^2}
\end{equation}

where $k$ is in W/(m-K), $T$ is in K, and the coefficients $a$ through $i$ depend on the selected RRR value.
Curve fit error is reported as approximately 1-2 percent relative to experimental data, varying by RRR value in [!cite](nist_copper).

### Specific Heat

Isobaric specific heat capacity is given as:

\begin{equation}
\log_{10}(C_p) = a + b\log_{10}(T) + c\log_{10}^2(T) + d\log_{10}^3(T) + e\log_{10}^4(T) + f\log_{10}^5(T) + g\log_{10}^6(T) + h\log_{10}^7(T)
\end{equation}

where $C_p$ is in J/(kg-K) and $T$ is in K. The coefficients are independent of RRR.
Curve fit error is reported as 10 percent for T $<$ 15 K and 5 percent for T $\ge$ 15 K in [!cite](nist_copper).

### Density

Density is assumed constant at 8940 kg/m$^3$ from [!cite](asm_copper).

## Range of Validity

The properties are valid for 4 K $\le$ T $\le$ 300 K.

!syntax parameters /SolidProperties/ThermalCopperProperties

!syntax inputs /SolidProperties/ThermalCopperProperties

!syntax children /SolidProperties/ThermalCopperProperties

!bibtex bibliography
