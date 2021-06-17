# Nomenclature

!table id=table:notation caption=List of variables and parameters used in documentation
| Symbol | Units | Physical description |
| --- | --- | --- |
| $A^{\kappa}$ | kg.kg$^{-1}$ | Mass of absorbed species $\kappa$ per mass of rock grain material |
| $\mathcal{A}^{i}_{\beta}$ | symbolic | Chemical species $i$ in phase $\beta$ |
| $A_{m}$ | m$^{2}$.L$^{-1}$ | specific reactive surface area for mineral $m$ |
| $\alpha_{\beta,L}$ | m | Longitudional dispersivity of phase $\beta$ |
| $\alpha_{\beta,T}$ | m | Transverse dispersivity of phase $\beta$ |
| $\alpha_{B}$ | dimensionless | Biot coefficient |
| $\alpha_{B}'$ | dimensionless | Biot coefficient that multiplies the porepressure term in the porosity.  Usually equal to $\alpha_{B}$, but may be $\neq\alpha_{B}$ for fracture-flow |
| $\alpha_{f}$ | K$^{-1}$ | Volumetric coefficient of thermal expansion of the fluid  |
| $\alpha_{T}$ | K$^{-1}$ | Volumetric coefficient of thermal expansion of the drained porous skeleton (ie, the porous rock without fluid, or which a fluid that is free to move in and out of the rock) |
| ${\mathbf b}$ | m.s$^{-2}$ | External force density acting on the porous solid.  This could be gravitational acceleration, or a load-density from a platten |
| $\beta$ | dimensionless | Index representing phase.  For example, $\beta$ might parameterise liquid ($\beta=0$), gas ($\beta=1$) and NAPL ($\beta=2$) |
| $C_{R}$ | J.kg$^{-1}$.K$^{-1}$ | Specific heat capacity of rock grains |
| $C_{ijkl}$ | Pa | Drained compliance tensor of the porous solid (ie, inverse of $E_{ijkl}$) |
| $C_{\beta}^{i}$ | moles per litre | concentration of species $i$ |
| $C_{v}$ | J.kg$^{-1}$.K$^{-1}$ | Fluid specific heat capacity at constant volume |
| $C_{p}$ | J.kg$^{-1}$.K$^{-1}$ | Fluid specific heat capacity at constant pressure |
| $\chi_{\beta}^{\kappa}$ | kg.kg$^{-1}$ | Mass fraction of component $\kappa$ present in phase $\beta$ |
| ${\mathcal{D}}_{\beta}^{\kappa}$ | m$^{2}$.s$^{-1}$ | Fluid dispersion tensor for species $\kappa$ in phase $\beta$ |
| $D_{\beta,L}^{\kappa}$ | m$^{2}$.s$^{-1}$ | Longitudional dispersion coefficient for species $\kappa$ in phase $\beta$ |
| $D_{\beta,T}^{\kappa}$ | m$^{2}$.s$^{-1}$ | Transverse dispersion coefficient for species $\kappa$ in phase $\beta$ |
| $d_{\beta}^{\kappa}$ | m$^{2}$.s$^{-1}$ | Molecular diffusion coefficient for component $\kappa$ in phase $\beta$ |
| $\delta_{l \beta}$ | dimenionless | Kronecker delta, unity if $\beta=l$, else zero |
| $\mathcal{E}$ | J.m$^{-3}$ | Energy density of the rock-fluid system |
| $\mathcal{E}_{\beta}$ | J.kg$^{-1}$ | Internal energy of fluid phase $\beta$ |
| $\epsilon_{ij}$ | dimensionless | Strain tensor of the porous solid ($(\nabla_{k}u_{l} + \nabla_{l}u_{k})/2$) |
| $\epsilon^{\mathrm{elastic}}_{ij}$ | dimensionless | Elastic strain tensor of the porous solid.  The total strain $\epsilon = \epsilon^{\mathrm{elastic}} + \epsilon^{\mathrm{plastic}}$ |
| $\epsilon^{\mathrm{plastic}}_{ij}$ | dimensionless | Plastic strain tensor of the porous solid.  The total strain $\epsilon = \epsilon^{\mathrm{elastic}} + \epsilon^{\mathrm{plastic}}$ |
| $\eta$ | dimensionless | exponent in rate expression
| $E_{ijkl}$ | Pa | Drained elasticity tensor of the porous skeleton (ie, this enters the stress-strain relation when fluid is allowed to freely drain from the skeleton, or when the skeleton is dry) |
| $\phi$ | dimensionless | Porosity of the solid |
| $\phi_{m}$ | dimensionless | Volume fraction of mineral in solid |
| $\mathbf{F}$ | kg.s$^{-1}$.m$^{-2}$ | Fluid flux.  This is a sum of the advective (Darcy) flux, and a diffusive-and-dispersive flux |
| $\mathbf{F}^{T}$ | J.s$^{-1}$.m$^{-2}$ | Heat flux.  This is a sum of heat conduction through the rock-fluid system, and convection with the fluid |
| ${\mathbf g}$ | m.s$^{-2}$ | Acceleration due to gravity.  It is a vector pointing downwards (eg $(0, 0, -9.81)$) |
| $\gamma_{\beta}^{i}$ | dimensionless | Activity coefficient of species $i$ in phase $\beta$ |
| $h_{\beta}$ | J.kg$^{-1}$ | Specific enthalpy of fluid phase $\beta$ |
| $H_{\kappa}$ | Pa | Henry coefficient for species $\kappa$ which describes the solubility of the species in the aqueous phase |
| $I_{m}$ | mol.L$^{-1}$.s$^{-1}$ | Mineral reaction rate |
| $I_{\mathrm{chem}}$ | kg.m$^{-3}$.s$^{-1}$ | Chemical precipitation and dissolution rate |
| $\kappa$ | dimensionless | Index representing species.  For example, $\kappa$ might parameterise water ($\kappa=0$), air ($\kappa=1$), and H$_{2}$ ($\kappa=2$).  It parameterises things that cannot be decomposed into other species, but can change phase.  For instance, sometimes it might be appropriate to consider air as a single species, while at other times it might be appropriate to consider it to be a mixture of nitrogen and oxygen ($\kappa=0$ and $\kappa=1$, say) |
| $k$ | m$^{2}$ | Permeability tensor of rock |
| $k_{\mathrm{r,}\beta}$ | dimensionless | Relative permeability of phase $\beta$. This is a nonlinear function of the independent variables.  Often it is just a function of the phase's saturation, but with Klinkenberg effects it will be a function of the gas pressure too.  In the single-phase, fully-saturated case it is unity  |
| $K$ | Pa | Bulk modulus of the drained porous skeleton.  In the anisotropic situation $1/K = = \delta_{ij}\delta_{kl}C_{ijkl}$ |
| $K_{\beta}^{i}$ | depends on reaction | equilibrium constant for secondary species $i$ in phase $\beta$ |
| $K_{f}$ | Pa | Bulk modulus of the fluid  |
| $k_{m}$ | mol.m$^{-2}$.s$^{-1}$ | Mineral rate constant |
| $\Lambda$ | s$^{-1}$ | Radioactive decay rate of a fluid species |
| $\lambda$ | J.s$^{-1}$.m$^{-1}$.K$^{-1}$ | Thermal conductivity of the rock-fluid system (J.s$^{-1}$.m$^{-1}$.K$^{-1} =$ kg.m.s$^{-3}$.K$^{-1}$).  It is a tensorial quantity to allow modelling of anisotropic situations, and is a function of the rock and fluid-phase's thermal conductivities |
| $\lambda_{dry}$ | J.s$^{-1}$.m$^{-1}$.K$^{-1}$ | Thermal conductivity of the rock-fluid system when aqueous phase saturation is zero |
| $\lambda_{wet}$ | J.s$^{-1}$.m$^{-1}$.K$^{-1}$ | Thermal conductivity of the rock-fluid system when aqueous phase saturation is unity |
| $M$ | kg.m$^{-3}$ | Mass density |
| $\mu$ | Pa.s | Dynamic viscosity measured in Pa.s or kg.m$^{-1}$.s$^{-1}$.  This is a nonlinear function of the independent variables |
| $\nu$ | dimensionless | Fraction of plastic-deformation energy that becomes heat energy.  Probably $\nu=1$ is correct |
| $\nu^{ij}_{\beta}$ | dimensionless | Stoichiometric coefficient of basis species $j$ in equilibrium reaction for secondary species $j$ |
| $\nabla$ | m$^{-1}$ | Spatial differential operator |
| $\Omega_{m}$ | dimensionless | mineral saturation ratio |
| $P$ | Pa | Fluid porepressure |
| $P_{f}$ | Pa | Measure of porepressure used in the effective stress. Often this is chosen to be $\sum_{\beta}S_{\beta}P_{\beta}$ |
| $P_{L}$ | Pa | Langmuir pressure |
| $\Phi_{\beta}^{j}$ | moles per litre | Total concentration of basis species $j$ in phase $\beta$ |
| $q$ | kg.m$^{-3}$.s$^{-1}$ | Fluid source |
| $q^{j}$ | mol.L$^{-1}$.s$^{-1}$ | Source of chemical species |
| $q^{T}$ | J.m$^{-3}$.s$^{-1}$ | Heat source |
| $\rho$ | kg.m$^{-3}$ | Fluid density |
| $\rho_{R}$ | kg.m$^{-3}$ | Grain density of the rock (so that $(1-\phi)\rho_{R}$ is the density of the dry porous rock) |
| $\rho_{\mathrm{mat}}$ | kg.m$^{-3}$ | The mass-density of the fluid-filled porous solid $\rho_{\mathrm{mat}} = (1 - \phi)\rho^{R} + \phi\sum_{\beta}S_{\beta}\rho_{\beta}$ |
| $\rho_{L}$ | kg.m$^{-3}$ | Langmuir density|
| $S$ | dimensionless | Saturation |
| $S_{l}$ | dimensionless | Saturation of aqueous phase |
| $\sigma^{\mathrm{tot}}_{ij}$ | Pa | Total stress.  An externally applied mechanical force will create a nonzero $\sigma^{\mathrm{tot}}$, and conversely, resolving $\sigma^{\mathrm{tot}}$ into forces yields the forces on nodes in the finite-element mesh |
| $\sigma^{\mathrm{eff}}_{ij}$ | Pa | Effective stress |
| $t$ | s | Time |
| $T$ | K | Temperature |
| $\tau_{0}\tau_{\beta}$ | dimensionless | The phase tortuosity, which includes a porous-medium dependent factor $\tau_{0}$ and a coefficient $\tau_{\beta}=\tau_{\beta}(S_{\beta})$ |
| $\tau_{L}$ | s | Langmuir desorption time constant |
| $\theta$ | dimensionless | Exponent in rate expression |
| ${\mathbf{u}}$ | m.s$^{-1}$ | Deformation vector of the porous solid |
| $\mathbf{v}$ | m.s$^{-1}$ | Darcy velocity (volume flux) |
| $\mathbf{v}_{s}$ | m.s$^{-1}$ | Velocity of the solid = $\partial \mathbf{u}/\partial t$, where $\mathbf{u}$ is the solid mechanical displacement vector of the porous solid
| $\overline{V}_{m}$ | L.mol$^{-1}$ | molar volume |
