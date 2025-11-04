# ShellResultantsAux

!syntax description /AuxKernels/ShellResultantsAux

!alert note
The three Cartesian local vectors for each shell element are indexed as follows: the first vector is indexed by 0, the second vector by 1, and the normal vector by 2. The convention used to define the direction of these vectors is explained in [ShellElements](/ShellElements.md)

The following stress resultants are computed using this auxiliary kernel:

axial_force_0: The in-plane axial force in the direction of the first local coordinate axis:

  $$F_{0} = \int_{-t/2}^{t/2} \sigma_{00} dz$$


axial_force_1: The in-plane axial force in the direction of the second local coordinate axis:

 $$ F_{1} = \int_{-t/2}^{t/2} \sigma_{11} dz$$


normal_force: The normal force applied to the shell element in the thickness direction. This force is expected to be always zero due to the plane stress assumption used in the shell element formulation, which disregards out-of-plane stresses:

  $$F_{N} = \int_{-t/2}^{t/2} \sigma_{22} dz$$


bending_moments_0: The bending moment about the first local coordinate axis:

  $$M_{0} = \int_{-t/2}^{t/2} \sigma_{11} z dz$$


bending_moment_1: The bending moment about the second local coordinate axis:

 $$ M_{1} = \int_{-t/2}^{t/2} \sigma_{00} z dz$$


bending_moment_01: The in-plane bending moment:

$$  M_{01} =M_{10}= \int_{-t/2}^{t/2} \sigma_{01} z dz$$


shear_force_01: The in-plane shear force:

 $$ Q_{01} =Q_{10}= \int_{-t/2}^{t/2} \sigma_{01} dz$$


shear_force_02: The transverse shear force:

$$Q_{02} =Q_{20}= \int_{-t/2}^{t/2} \sigma_{02} dz$$


shear_force_12: The transverse shear force:

$$  Q_{12} =Q_{21}= \int_{-t/2}^{t/2} \sigma_{12} dz$$




## Example Input Syntax

!listing modules/solid_mechanics/test/tests/shell/static/plate_cantilever.i block=AuxKernels/moment_22

!syntax parameters /AuxKernels/ShellResultantsAux

!syntax inputs /AuxKernels/ShellResultantsAux

!syntax children /AuxKernels/ShellResultantsAux
