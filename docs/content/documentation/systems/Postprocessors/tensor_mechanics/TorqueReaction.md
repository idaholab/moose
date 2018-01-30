# Torque Reaction
!syntax description /Postprocessors/TorqueReaction

## Description
The postprocessor `TorqueReaction` calculates the applied torque from the reation forces, coupled as AuxVariables, about a user specified axis of rotation.

TorqueReaction takes a scalar approach to calculating the sum of the acting torques by projecting both the reaction force vector and the position vector (the coordinates of the node upon which the force is applied) onto the axis of rotation and applying the Pythagorean theorem, as in a statics course.
This scalar approach allows the postprocessor to accept any axis of rotation direction.

The torque from the reaction forces is calculated, as shown in \eqref{eq:calculate_torque_vector} relative to the user specified axis of rotation origin and direction.
\begin{equation}
\label{eq:calculate_torque_vector}
  \begin{aligned}
    \boldsymbol{\tau} & = \hat{P}_n \times \mathcal{f} \\
    \hat{P}_n & = \left( P_c - P_o \right) - \frac{\left( P_c - P_o \right) \ cdot d}{|d|^2} \cdot d
  \end{aligned}
\end{equation}
where $\mathcal{f}$ is the applied reaction force vector, $P_c$ is the current node position, $P_o$ is the origin of the axis of rotation, and $d$ is the direction vector of the axis of rotation.
The component of the torque acting along the user specified axis of rotation vector, $\tau_c$ is calcuated as \eqref{eq:calculate_torque_component}:
\begin{equation}
\label{eq:calculate_torque_component}
  \tau_c = \frac{\boldsymbol{\tau} \cdot d}{|d|^2} \ cdot d
\end{equation}
where $\boldsymbol{\tau}$ is the torque vector calculated in \eqref{eq:calculate_torque_vector} and $d$ is the axis of rotation direction vector.

## Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/torque_reaction/torque_reaction_tm.i block=Postprocessors/torque

A list of the AuxVariables which save the reaction forces must be provided to the `TorqueReaction` block.
!listing modules/tensor_mechanics/test/tests/torque_reaction/torque_reaction_tm.i block=AuxVariables/saved_x

!listing modules/tensor_mechanics/test/tests/torque_reaction/torque_reaction_tm.i block=AuxVariables/saved_y

The reaction force AuxVariables must also be connected to the stress divergence kernel through the `save_in` parameter
!listing modules/tensor_mechanics/test/tests/torque_reaction/torque_reaction_tm.i block=Kernels/TensorMechanics

!syntax parameters /Postprocessors/TorqueReaction

!syntax inputs /Postprocessors/TorqueReaction

!syntax children /Postprocessors/TorqueReaction
