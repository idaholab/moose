# Compute Updated Euler Angle

## Description

This class computes the updated Euler angle for crystal plasticity simulations. This needs to be used together with the  [ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md) class, where the updated rotation matrix is computed.

## Example Input File Syntax

To use this class, the user will need to add an additional block under the `materials` block for calculating the updated Euler angles:

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/stress_update_material_based/update_euler_angle.i block= Materials

!alert note
This needs to be used in combination of the material models that are inherited from `CrystalPlasticityStressUpdateBase` and work under [ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md).

## Verification

We verify the Euler angles calculation in this class by simulating uniaxial tension along the z-direction for three different crystals that have different initial orientations, $[111]$, $[112]$, and $[123]$. Here, we simulate the deformation of FCC crystals using the Kalidindi [!citep](kalidindi1992) crystal plasticity model (see the model's documentation page [Here](/CrystalPlasticityKalidindiUpdate.md)).

Below are the inverse pole figures for the three cases. It can be seen that the $[111]$ oriented crystal pole does not move in the inverse pole figure during deformation.  A perfectly oriented $[112]$ is initially situated on the symmetry line in the inverse pole figure. When this crystal is pulled, the pole moves along the symmetry line towards $[111]$. For the $[123]$ crystal, the crystal tensile axis is initially inside the inverse pole figure and moves towards $[111]$ when deformed in tension.

!media tensor_mechanics/crystal_plasticity/Inverse_Pole_Fig.png
    id=IPF_Orients
    caption=Inverse pole figure of a singe crystal undergone uniaxial tension. Three cases with different initial orientations are demonstrated. The initial orientations are $[111]$, $[112]$, and $[123]$ from the left to the right.
    style=display:block;margin-left:auto;margin-right:auto;width:90%

The inverse pole figures are plotted using MATLAB tool box [MTEX](https://github.com/mtex-toolbox/mtex). The above results can be reproduced  by running [this test](modules/tensor_mechanics/test/tests/crystal_plasticity/stress_update_material_based/update_euler_angle.i) with end_time = 50 (seconds).

The MOOSE developer would like to acknowledge contributions from Dr. Alankar Alankar and Ritam Chatterjee ([IMaGen](https://www.me.iitb.ac.in/~alankar/), Indian Institute of Technology, Bombay, India) to this verification case.


!syntax parameters /Materials/ComputeUpdatedEulerAngle

!syntax inputs /Materials/ComputeUpdatedEulerAngle

!syntax children /Materials/ComputeUpdatedEulerAngle
