# CircularBoundaryCorrectionGenerator

!syntax description /Mesh/CircularBoundaryCorrectionGenerator

## Overview

The `CircularBoundaryCorrectionGenerator` object performs radius correction to preserve circular area considering polygonization effect for full or partial circular boundaries in a 2D mesh within the `Z=0` plane.

In a 2D mesh, a "circular boundary" consists of sides that connect a series of nodes on the circle, which actually form a polygon boundary. Due to the polygonization effect, the area within a "circular boundary" in a 2D mesh is actually smaller than a real circle with the same radius. Such a discrepancy could cause issues in some simulations that involve physics that are sensitive to volume conservation.

Therefore, a corrected radius can be used to generate the "circle-like" polygon to enforce that the polygon area is the same as the original circle without polygonization. This can be achieved with either of the following approaches.

### Moving Radial Nodes

The most straightforward approach for circular correction is to move the nodes on the circular boundary in their respective radial directions (see the left sub-figure of [schematic]). In that case, the azimuthal angle intervals of the boundary sides do not change. Therefore, the algorithm is relatively simple. This is also the default approach of this mesh generator.

For a polygon region used to mesh a full or partial circle, each side (with index $i$) corresponds to an azimuthal angle interval, $\theta_i$ , which is defined by the side and the center of the polygon:

!equation id=polygon_area
S_{polygon}=\frac12r_{polygon}^2\Sigma_i~\sin \theta_i

with

!equation id=azi
\Theta_{total}=\Sigma_i~\theta_i

$\Theta_{total}$ should be $2\pi$ for a full circle and between 0 to $2\pi$ for a partial circle. For such a full or partial real circle, the area has the following form:

!equation id=circle_area
S_{circle}=\frac{\Theta_{total}}{2}r_{circle}^2=\frac12r_{circle}^2\Sigma_i~\theta_i

To enforce that $S_{polygon}=S_{circle}$, the radii of the polygon and the circle have the following relation,

!equation id=relation
r_{polygon}=\sqrt{\frac{\Sigma_i~\theta_i}{\Sigma_i~\sin \theta_i}}r_{circle}=f_{corr}r_{circle}

Where $f_{corr}$ is the correction factor used in this object to ensure volume preservation.

!media mesh_modifiers/circular_correction.png
      style=display: block;margin-left:auto;margin-right:auto;width:60%;
      id=schematic
      caption=A schematic drawing showing the two approaches to correct the polygonization effect for a partial circular boundary

### Optional additional circular boundary expansion in the span direction

Moving the radial nodes is undoubtedly the most generalized approach for a full circular boundary. However, for a partial circular boundary, moving the two end nodes in their radial directions may deform the original shape. Therefore, moving the end nodes in the span direction of the partial circular boundary (i.e., arc) provides an alternative approach (see the right subfigure of [schematic]). Moving the end nodes in the span direction inevitably changes the azimuthal angle intervals. To make this change consistent for all the boundary sides, a scaling coefficient, $c$, is applied to every $\theta_i$.

!equation id=scaling_theta
\theta_i^{corr}=c\theta_i

!equation id=span_relation
f=\frac{r_{polygon}}{r_{circle}}=\frac{\cos\left(\Sigma_i~\theta_i/2\right)}{\cos\left(c\Sigma_i~\theta_i/2\right)}=\sqrt{\frac{\Sigma_i~\theta_i-\sin\left(\Sigma_i~\theta_i\right)}{\Sigma_i~\sin\left(c\theta_i\right)-\sin\left(c\Sigma_i~\theta_i\right)}}

!equation id=span_relation_2
f^2=\frac{1+\cos\left(\Sigma_i~\theta_i\right)}{1+\cos\left(c\Sigma_i~\theta_i\right)}=\frac{\Sigma_i~\theta_i-\sin\left(\Sigma_i~\theta_i\right)}{\Sigma_i~\sin\left(c\theta_i\right)-\sin\left(c\Sigma_i~\theta_i\right)}

 Note that if the partial circular boundary happens to be a half circle (i.e., $\Sigma_i~\theta_i=\pi$), [span_relation_2] cannot be solved as both numerator and denominator are zero. In that case the span direction is also the radial direction. Therefore, $c$ has a trivial value of unity; and the node moving should be calculated using the "radial nodes moving" approach. If the boundary if not a half circle (i.e., $\Sigma_i~\theta_i\neq\pi$), the above equation is solved by Newton-Raphson method to obtain $c$. The displacement of the end nodes ($e_{end}$) can be calculated as follows.

!equation id=span_disp
e_{end}=r_{polygon}\sin\left(\frac12c\Sigma_i~\theta_i\right)-r_{circular}\sin\left(\frac12\Sigma_i~\theta_i\right)

Users should set [!param](/Mesh/CircularBoundaryCorrectionGenerator/move_end_nodes_in_span_direction) as `true` to enable this radius correction approach for partial circular boundaries.

## Usage

This object is capable of correcting multiple circular boundaries within a 2D mesh at the same time. Each circular boundary must be provided as a single boundary id/name in [!param](/Mesh/CircularBoundaryCorrectionGenerator/input_mesh_circular_boundaries).

Moving the nodes on a circular boundary may flip the local elements. In order to reduce the probability of element flipping, a transition area is defined for each circular boundary, which is a ring area covering both inner and outer sides of the circular boundary in which the nodes will be moved based on the correction factor. The moving distance fades as the node-to-circle distance increases. The transition area size is defined using a fraction of the boundary radius, which can be customized in [!param](/Mesh/CircularBoundaryCorrectionGenerator/transition_layer_ratios).

## Example Syntax

!listing /test/tests/meshgenerators/circular_correction_generator/radius_corr.i block=Mesh

!syntax parameters /Mesh/CircularBoundaryCorrectionGenerator

!syntax inputs /Mesh/CircularBoundaryCorrectionGenerator

!syntax children /Mesh/CircularBoundaryCorrectionGenerator
