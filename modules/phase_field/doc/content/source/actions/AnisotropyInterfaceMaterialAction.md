# AnisotropyInterfaceMaterialAction

!syntax description /Modules/PhaseField/AnisotropyInterface/AnisotropyInterfaceMaterialAction

This action simplifies the creation of [`InterfaceOrientationMultiphaseMaterial`](/InterfaceOrientationMultiphaseMaterial.md) which is needed for anisotropic properties on the interface kernel. Each interface consists of two variables, but when multiple order parameters are present, each pair of order parameters requires its own [`InterfaceOrientationMultiphaseMaterial`](/InterfaceOrientationMultiphaseMaterial.md). Having N order parameters requires N(N-1) interface materials, and given each order parameters may have its own orientation means the misorientation between two order parameters vary considerably.

### Anisotropic property Overview

Anisotropy on the interface can be introduced by considering the normal of the interface between two abutting order parameters. Classic phase field model uses:
\begin{equation}
\kappa_{ij} = \kappa_{\circ} \left[ 1 + \delta cos(q(\theta_{ij} - \theta_{\circ}))\right]^2
\end{equation}
where $\kappa_{ij}$ is the anisotropic gradient energy coefficient between order parameter $i$ and $j$, $\kappa_{\circ}$ is the average gradient energy coefficient, $\delta$ is the strength of anisotropic, $q$ is the mode number (e.g. 4 produces four-sided grain), $\theta_{\circ}$ is the reference angle between two order parameters, and \theta_{ij} is the inclination angle relative to the x-axis. 

Computing $\theta_{\circ}$ requires the normal of the interface between two abutting order parameters, which can be computed with:
\begin{equation}
\hat{n}_{ij} = \frac{\nabla \eta_i - \nabla \eta_j}{|\nabla \eta_i - \nabla \eta_j|}
\end{equation}
where $\hat{n}_{ij}$ is the unit normal of the interface between order parameter $i$ and $j$, and $\nabla \eta$ is the gradient of the order parameter. The inclination angle between two order parameters are then computed with:
\begin{equation}
\theta_{ij} = arctan \left( \frac{n_y}{n_x} \right)
\end{equation}
where $n_y$ and $n_x$ is the y-component and x-component of the interfacial normal.

Additional complexity where each order parameter has its own orientation requires the misorientation of each, that is the difference between the two orientation, i.e. $\theta_{\circ} = \theta_i - \theta_j$.

### Output Overview

When using this action, the naming of each materials becomes important since it will determine which material belong to which kernel. Thus, to make it easier, the output of each material is designed to be "kappa_name_" + "etaa" + "etab" in which "etaa" is the main variable and "etab" is the variable coupled to "etaa". For example, given two order parameters, eta1 and eta2, and this action input parameters set to "kappa_name" = mob, "dkappadgrad_eta_name" = deriv_mob, and "d2kappadgrad_eta_name" = deriv2_mob, the material assigned to eta1 kernel will have the name "mob_eta1_eta2" with its derivatives, "deriv_mob_eta1_eta2" and "deriv2_mob_eta1_eta2" while the material assigned to eta2 kernel will have "mob_eta2_eta1" with its derivatives, "deriv_mob_eta2_eta1" and "deriv2_mob_eta2_eta1". This is because the order of order parameter becomes important if each order parameter has different orientation. If eta1 has orientation of 15 while eta2 has orientation of 0, "mob_eta2_eta1" has its reference_angle set to -15 while "mob_eta1_eta2" has its reference_angle set to 15. 

### Combining all anisotropic material

Following [!cite](moelans_quantitative_2008), it is possible to combine all InterfaceOrientationMultiphaseMaterial into one material property, in which:

\begin{equation}
\kappa(\theta_{ij}, \theta_{\circ}) = \frac{\sum_i=1^N \sum_{j>i}^N \kappa_{ij} \eta_i^2 \eta_j^2}{\sum_i=1^N \sum_{j>i}^N \eta_i^2 \eta_j^2}
\end{equation}

When combining all equations into one, its derivatives will also need to be combined into one with the same equation as above.

!syntax parameters /Modules/PhaseField/AnisotropyInterface/AnisotropyInterfaceMaterialAction

!bibtex bibliography