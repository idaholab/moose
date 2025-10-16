# Inclined Crack in Infinite Plate

The following example follows from [!citet](Dolbow99,Richardson2011) where the stress intensity factors for a plate with an angled center crack is subjected to a far-field load in the y-direction.   The analytic solution for the stress intensity factors of the configuration shown in [stress] is given by

\begin{equation}
\begin{aligned}
    KI &= \sigma\sqrt{\pi a} \cos^2(\theta) \\
    KII &= \sigma\sqrt{\pi a} \cos(\theta)\sin(\theta)
\end{aligned}
\end{equation}

where the crack angle with respect to the x-axis is $\theta$, the crack length is $a$ and the far-field load $\sigma$.  Convergence with global mesh refinement to the analytic solution is shown for a range of crack orientations in [convergence].  The values for $KI$ in blue show better convergence than $KII$ shown in red for mixed mode loading of the crack-tip when $20^\circ\ge\theta\le80^\circ$.  [qintegral] shows the q-function for integrating the J-integral.  Incorrectly specifying the radii for the q-function will lead to inaccurate calculation of the J-integral and stress intensity factors.  The q-function should exclude a few elements around the crack tip where $q=1$.  The q-function radii are designated in the `DomainIntegral` block shown in [input].  This example uses a cutter mesh, shown by the white line in [stress], to create the initial crack using the `MeshCut2DFractureUserObject` object, also shown in [input].

!media large_media/xfem/stress_field_label.png
      style=width:80%;margin:auto;padding-top:2.5%;background-color: white;color: black;
      id=stress
      caption= Left: Inclined plate simulation domain showing the stress field in the y-direction.  The mesh dimensions are $W\=H\=40$ and the crack length is $2a\=2$.  The white line in the center is the inclined crack created by the XFEM cutter mesh.  Right: Close up of crack geometry with labels.

!media large_media/xfem/q1_field_label.png
      style=width:70%;margin:auto;padding-top:2.5%;background-color: white;color: black;
      id=qintegral
      caption=Domain integral integration field (q-function) around the bottom crack-tip.  The integration field is integrated over the area between r_inner and r_outer specified in the `DomainIntegral` block in [input].  The initial XFEM cut created by the `MeshCut2DFractureUserObject` is also shown.

!media large_media/xfem/incline_angle_convergence.png
      style=width:70%;margin:auto;padding-top:2.5%;background-color: white;color: black;
      id=convergence
      caption=Convergence plot for KI and KII for mesh size ranging from h\=0.2, 0.1, 0.05.  The results shown [!ref](stress) and [!ref](qintegral) are for h\=0.1.


!listing xfem/test/tests/mesh_cut_2D_fracture/inclined_center_crack.i  id=input block=UserObjects DomainIntegral caption=Input file for inclined crack.

!bibtex bibliography
