# Contact Module

The interaction of moving bodies is a common occurrence in our world, and therefore modeling such problems is essential to accurately represent the mechanical behavior of the physical world. However, finite element methods do not have an inherent means of modeling contact. Therefore, specific contact algorithms are required. These algorithms enforce constraints between surfaces in the mesh, to prevent penetration and develop contact forces. The MOOSE contact module provides the necessary tools for modeling mechanical contact.

[](---)

## Theory

Mechanical contact between two deformable bodies is based on three requirements.

\begin{equation*}
g \le 0,
\end{equation*}
\begin{equation*}
t_N \ge 0,
\end{equation*}
\begin{equation*}
t_N g = 0.
\end{equation*}


That is, the penetration distance (typically referred to as the gap $g$ in the contact literature) of one of the body into another must not be positive; the contact force $t_N$ opposing penetration must be positive in the normal direction; and either the penetration distance or the contact force must be zero at all times.  In the MOOSE Contact Module, these contact constraints are enforced through the use of either node/face constraints or by using a mortar method.

[](---)

## Node/Face Mechanical Contact

Contact constraints can be enforced through the use of node/face constraints in a manner similar to that detailed by [!cite](heinstein_algorithm_1999)). In this approach, first, a geometric search determines which secondary nodes have penetrated primary faces. For those nodes, the internal force computed by the divergence of stress is moved to the appropriate primary face at the point of contact. Those forces are distributed to primary nodes by employing the finite element shape functions. Additionally, the secondary nodes are constrained to remain on the primary faces, preventing penetration. The module currently supports frictionless, frictional, and glued contact.

[](---)

## Mortar-Based Mechanical Contact

Models specific for mechanical contact enforcement have been developed based on the MOOSE
[mortar constraint system](Constraints/index.md), and provide an alternative
discretization technique for solving mechanical contact. Results of performance studies
using this approach are summarized in [MortarPerformance](modules/contact/MortarPerformance.md).

[](---)

!row!
!col! small=8 medium=4 large=5 icon=device_hub

### Tutorial and examples class=center style=font-weight:200;

!include modules/contact/contact_examples.md

!col-end!

!col! small=8 medium=4 large=5 icon=storage

### Implementation details and analysis class=center style=font-weight:200;

- [Mortar performance](contact/MortarPerformance.md)


!col-end!
!row-end!

## `Contact` Syntax Block

Setting up a model to use contact enforcement in MOOSE requires the creation of
multiple types of MOOSE objects. Using the top-level
[Contact](/Contact/index.md) syntax block, which streamlines the process of
setting up these objects, is highly recommended, and supports most available types of contact.
The following input file example shows the basic usage of the `Contact` block:

!listing test/tests/sliding_block/sliding/frictionless_kinematic.i block=Contact

## Objects, Actions, and Syntax

!syntax complete groups=ContactApp level=3
