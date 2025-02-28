# LeadLithiumFluidProperties

!syntax description /FluidProperties/LeadLithiumFluidProperties

These properties are based on experiments and compilations reported in the literature, e.g., \citep{Schulz1991,Hubberstey1992,Giancarli1996,Zinkle1998}. Most properties depend only on temperature; the fluid is considered incompressible over the range of interest. The fluid properties are summarized in Table \ref{tab:leadli}, which reports the formulas used and their origin.

!table id=tab:leadli caption=Table of properties, equations, and references for the Lead–Lithium fluid properties.
| Properties                             | Equations | Reference |
| :------------------------------------- | :-------- | :-------- |
| Melting point, $T_{mo}$ (K)            | $508$ | $\citep{Hubberstey1992}$ |
| Density, $\rho$ (kg/m$^3$)             | $\displaystyle 10520.35 - 1.19051\,T$ | $\citep{MasdeLesValls2008}$ |
| Viscosity, $\mu$ (Pa$\cdot$s)           | $\displaystyle 1.87 \times 10^{-4}\,\exp\!\left(\frac{11640}{8.314\,T}\right)$ | $\citep{Schulz1991}$ |
| Specific enthalpy, $h$ (J/kg)            | $\displaystyle 195\,(T-T_{mo}) - 0.5\times9.116\times10^{-3}\,(T^2-T_{mo}^2)$ | $\citep{Zinkle1998}$ |
| Thermal Conductivity, $k$ (W/m-K)      | $\displaystyle 9.144 + 0.019631\,T$ | $\citep{MasdeLesValls2008}$ |
| Isobaric Specific Heat, $c_p$ (J/kg-K)   | $\displaystyle 195 - 9.116\times10^{-3}\,T$ | $\citep{Schulz1991}$ |
| Isochoric Specific Heat, $c_v$ (J/kg-K)  | $\displaystyle \frac{c_p}{1+\left(\frac{1.19051}{10520.35-1.19051\,T}\right)^2 \frac{B_S\,T}{\rho\,c_p}}$ | $\citep{Zinkle1998}$ |
| Isentropic Bulk Modulus, $B_S$ (Pa)      | $\displaystyle \left(44.73077 - 0.02634615\,T + 5.76923\times10^{-6}\,T^2\right)\times10^{9}$ | $\citep{Hubberstey1992}$ |
| Speed of Sound, $c$ (m/s)              | $\displaystyle 1959.63 - 0.306\,T$ | $\citep{Schulz1991}$ |


## Range of Validity

The properties defined in \texttt{LeadLithiumFluidProperties} are valid for

\[
508\,\mathrm{K} \le T \le 1800\,\mathrm{K},
\]

and for pressures near atmospheric up to a few MPa, where the assumption of incompressibility holds.

## Uncertainties of Lead–Lithium Fluid Properties

Based on experimental studies \citep{Schulz1991,Hubberstey1992} and compilations in fusion materials handbooks \citep{Giancarli1996,Zinkle1998}, the reported uncertainties for Lead–Lithium fluid properties are approximately:

\[
\begin{array}{|l|c|}
\hline
\textbf{Property} & \textbf{Uncertainty (\%)} \\
\hline
\text{Density} & \approx 1\% \\
\text{Viscosity} & \approx 10\% \\
\text{Thermal Conductivity} & \approx 15\% \\
\text{Isobaric Specific Heat} & \approx 7\% \\
\hline
\end{array}
\]

!syntax parameters /FluidProperties/LeadLithiumFluidProperties

!syntax inputs /FluidProperties/LeadLithiumFluidProperties

!syntax children /FluidProperties/LeadLithiumFluidProperties

!! Rather than pollute the bib file, let's have the references here

## References

```
@article{MasdeLesValls2008,
  author    = {Mas de Les Valls, J. and Others},
  title     = {Thermophysical Properties of Liquid Pb–Li Alloys for Fusion Applications},
  journal   = {Journal of Nuclear Materials},
  volume    = {376},
  number    = {1--3},
  pages     = {353--357},
  year      = {2008},
  doi       = {10.1016/j.jnucmat.2008.02.057}
}

@article{Schulz1991,
  author    = {Schulz, B. and Lau, M. K. and McFarlane, P. N.},
  title     = {Thermophysical Properties of the Li(17)Pb(83) Eutectic Alloy for Fusion Reactor Applications},
  journal   = {Fusion Engineering and Design},
  volume    = {14},
  pages     = {199--205},
  year      = {1991},
  doi       = {10.1016/0920-3796(91)90083-7}
}

@article{Hubberstey1992,
  author    = {Hubberstey, O. and Galanakis, A. E. and van Loon, J. J. A.},
  title     = {Re-assessment of the Li–Pb Phase Diagram for Fusion Applications},
  journal   = {Journal of Nuclear Materials},
  volume    = {191},
  number    = {1--3},
  pages     = {282--290},
  year      = {1992},
  doi       = {10.1016/0022-3115(92)90041-K}
}

@techreport{Giancarli1996,
  author       = {Giancarli, L. and Others},
  title        = {Design and Fabrication of a Fusion Blanket Using Liquid Pb-17Li},
  institution  = {Commissariat \`a l'\'energie atomique (CEA)},
  year         = {1996},
  number       = {CEA-R--96-XXX},
  address      = {France}
}

@techreport{Zinkle1998,
  author       = {Zinkle, S. J.},
  title        = {Summary of Physical Properties for Pb-17Li as a Fusion Reactor Material},
  institution  = {Oak Ridge National Laboratory},
  year         = {1998},
  number       = {ORNL/TM-1341},
  address      = {Oak Ridge, TN, USA}
}
```
