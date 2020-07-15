# MechanicalContactConstraint

!syntax description /Constraints/MechanicalContactConstraint

## Description

The `MechanicalContactConstraint` class enforces mechanical contact of different types. Three contact models are available. These are `glued`, `frictionless`, and `coulomb`. Here we have
\begin{equation*}
\boldsymbol{t} = t_N\boldsymbol{n} + t_{T}\boldsymbol{\tau},
\end{equation*}
where $\boldsymbol{t}$ denote the total traction, $t_N$ is the normal traction, $t_{T}$ represents the tangential traction, $\boldsymbol{n}$ and $\boldsymbol{\tau}$ are the normal and tangential unit vectors, respectively. The normal contact constraint is the same for these three contact types and are detailed in [contact module description](/modules/contact/index.html). The differences among these contact types lie in the constraint equations for the tangential contact, which is described in the following section.

### Contact Models

The `glued` contact does not allow slip between the nodes once they come into contact, i.e., $|\boldsymbol{v}_{T}| = 0$,
where $\boldsymbol{v}_{T}$ is the tangential relative velocity.
 Due to the no-slip condition, the `glued` contact is often used to model bonded connections between adjacent bodies that may have mismatching meshes in solid mechanics.

In contrast to the `glued` contact, the `frictionless` contact allows slip between the nodes that are in contact. No tangential traction is assumed, i.e., $|\boldsymbol{v}_{T}| > 0$ and ${t}_{T} = {0}$. The `frictionless` contact is often utilized when the contact surface is pretty smooth thus tangential fraction along the interface can be omitted.

Different from the above two contact models, the `coulomb` model accounts for the friction along the interface caused by the relative motion between nodes. This is modeled using the Coulomb law, which computes a tangential traction that is proportional to the normal traction via a slip condition:
\begin{equation*}
{t}_{T} = -\mu |t_N| \frac{\boldsymbol{v}_T}{||\boldsymbol{v}_T||}\cdot\tau \quad \text{if} \quad ||{t}_T||> \mu |t_N|,
\end{equation*}
where $\mu$ is the friction coefficient.

### Contact Formulations

To enforce the contact constraints, three types of formulations are available. These are `kinematic`, `penalty`, and `mortar`.
The `kinematic` and `penalty` formulations have been developed for some time and turn out to be robust for many mechanical contact problems. Recent efforts have been put in developing a mortar-based contact formulation. In this documentation, however,  we focus on `kinematic` and `penalty` formulations. For readers who are interested in mortar-based mechanical contact, please go to [mortar constraint system](Constraints/index.md) for more information.

The overall workflow of imposing a mechanical constraint is as follows: 1) Determine whether penetration occurs. If so, constrain displacement; 2) In the normal direction, eliminate the penetration; 3) In the tangential direction, slip must satisfy friction model, no slip is allowed for the `glued` model, no constraint is applied for the `frictionless` model, Kuhn-Tucker conditions must be satisfied in the tangential direction for the `coulomb` model. The mechanical contact enforcement steps for the `glued` and `frictionless` models are summarized below:

For `penalty`:

|  Glued  | Frictionless |
| ---------- | ------------ |
|  $\boldsymbol{f}_c = k_p (\boldsymbol{x} - \boldsymbol{x}_{p_{t-1}})$     | $\boldsymbol{f}_c = \boldsymbol{n}\boldsymbol{n}^{T} k_p (\boldsymbol{x} - \boldsymbol{x}_p)$ |
|  $\boldsymbol{r}_s = \boldsymbol{r}_s + \boldsymbol{f}_c$    | $\boldsymbol{r}_s = \boldsymbol{r}_s + \boldsymbol{f}_c$ |
|  $\boldsymbol{r}_m = \boldsymbol{r}_m - \phi_i\boldsymbol{f}_c$     | $\boldsymbol{r}_m = \boldsymbol{r}_m - \phi_i \boldsymbol{f}_c$ |

For `kinematic`:

|  Glued  | Frictionless |
| ---------- | ------------ |
|  $\boldsymbol{f}_c = -\boldsymbol{r}_{s_{copy}}$     | $\boldsymbol{f}_c = -\boldsymbol{n}\boldsymbol{n}^{T} \boldsymbol{r}_{s_{copy}}$  |
|  $\boldsymbol{r}_s = \boldsymbol{r}_s+\boldsymbol{f}_c+k_p(\boldsymbol{x} - \boldsymbol{x}_{p_{t-1}})$     | $\boldsymbol{r}_s = \boldsymbol{r}_s + \boldsymbol{f}_c + \boldsymbol{n}\boldsymbol{n}^{T} k_p (\boldsymbol{x} - \boldsymbol{x}_{p_{t-1}})$ |
|  $\boldsymbol{r}_m = \boldsymbol{r}_m - \phi_i \boldsymbol{f}_c$     | $\boldsymbol{r}_m = \boldsymbol{r}_m - \phi_i \boldsymbol{f}_c$ |

Note that the `kinematic` method uses penalty parameter only to enforce the condition that the secondary node has the desired position on the primary face, so the errors are very small in the converged solution. Unlike in `penalty` formulations, penalty compliance is not introduced on the contact surface. Therefore, the converged solution has no error due to penalty compliance.

For the frictional contact (`coulomb` model), the `penalty` method calculates normal force and tangential predictor force using penalty stiffness and check for frictional capacity. While using the `kinematic` method, the deformation of surrounding material serves the role of the penalty parameter.

### Contact gap offset

Gap offset can be provided to the current mechanical contact constraint. It can be either `secondary_gap_offset` (gap offset from secondary side) or `mapped_primary_gap_offset` (gap offset from primary side but mapped to secondary side). Use of these gap offset parameters treats the surfaces as if they were virtually extended (positive offset value) or narrowed (negative offset value) by the specified amount, so that the surfaces are treated as if they are closer or further away than they actually are. There is no deformation within the material in this gap offset region.

!syntax parameters /Constraints/MechanicalContactConstraint

!syntax inputs /Constraints/MechanicalContactConstraint

!syntax children /Constraints/MechanicalContactConstraint
