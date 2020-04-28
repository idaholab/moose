# Torque Reaction

!syntax description /Postprocessors/TorqueReaction

## Description

The postprocessor `TorqueReaction` calculates the applied torque from the reaction forces, coupled as
AuxVariables, about a user specified axis of rotation.

TorqueReaction takes a scalar approach to calculating the sum of the acting torques by projecting
both the reaction force vector and the position vector (the coordinates of the node upon which the
force is applied) onto the axis of rotation and applying the Pythagorean theorem, as in a statics
course.  This scalar approach allows the postprocessor to accept any axis of rotation direction.

The torque from the reaction forces is calculated, as shown in [eq:calculate_torque_vector]
relative to the user specified axis of rotation origin and direction.
\begin{equation}
\label{eq:calculate_torque_vector}
  \begin{aligned}
    \boldsymbol{\tau} & = \hat{P}_n \times \mathcal{f} \\
    \hat{P}_n & = \left( P_c - P_o \right) - \frac{\left( P_c - P_o \right) \ cdot d}{|d|^2} \cdot d
  \end{aligned}
\end{equation}
where $\mathcal{f}$ is the applied reaction force vector, $P_c$ is the current node position, $P_o$
is the origin of the axis of rotation, and $d$ is the direction vector of the axis of rotation.  The
component of the torque acting along the user specified axis of rotation vector, $\tau_c$ is
calcuated as [eq:calculate_torque_component]:
\begin{equation}
\label{eq:calculate_torque_component}
  \tau_c = \frac{\boldsymbol{\tau} \cdot d}{|d|^2} \ cdot d
\end{equation}
where $\boldsymbol{\tau}$ is the torque vector calculated in [eq:calculate_torque_vector] and
$d$ is the axis of rotation direction vector.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/torque_reaction/torque_reaction.i
         block=Postprocessors/torque

A list of the AuxVariables which save the reaction forces must be provided to the `TorqueReaction` block.

!listing modules/tensor_mechanics/test/tests/torque_reaction/torque_reaction.i
         block=AuxVariables/saved_x

!listing modules/tensor_mechanics/test/tests/torque_reaction/torque_reaction.i
         block=AuxVariables/saved_y

The reaction force AuxVariables must also be computed using the tagging system to save the reactions to a separate vector in the stress divergence kernel, which is handled in this case using the `extra_vector_tags` parameter in the `Master` action:

!listing modules/tensor_mechanics/test/tests/torque_reaction/torque_reaction.i
         block=Modules/TensorMechanics/Master

The `TagVectorAux` AuxKernel is used to extract the saved components of the reaction vector from a tagged vector and put them in an AuxVariable. This shows how the $x$ component of this is extracted, and the others are handled in a similar manner:

!listing modules/tensor_mechanics/test/tests/torque_reaction/torque_reaction.i
         block=AuxKernels/saved_x

!syntax parameters /Postprocessors/TorqueReaction

!syntax inputs /Postprocessors/TorqueReaction

!syntax children /Postprocessors/TorqueReaction
