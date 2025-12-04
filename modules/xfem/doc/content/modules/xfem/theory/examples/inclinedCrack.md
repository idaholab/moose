# Inclined Crack in Infinite Plate

The following example follows from [!citet](Dolbow99,Richardson2011,belytschko_elastic_1999) where the stress intensity factors for a plate with an angled center crack is subjected to a far-field load in the y-direction.   The analytic solution for the stress intensity factors of the configuration shown in [!ref](stress) is given by

\begin{equation}
\label{eq:inclined}
\begin{aligned}
    K_I &= \sigma\sqrt{\pi a} \cos^2(\theta) \\
    K_{II} &= \sigma\sqrt{\pi a} \cos(\theta)\sin(\theta)
\end{aligned}
\end{equation}

where the crack angle with respect to the x-axis is $\theta$, the crack length is $a$ and the far-field load $\sigma$.  Convergence with global mesh refinement to the analytic solution is shown for a range of crack orientations in [!ref](convergence).  The values for $K_I$ in blue show better convergence than $K_{II}$ shown in red for mixed mode loading of the crack-tip when $20^\circ\ge\theta\le80^\circ$.  [!ref](qintegral) shows the q-function for integrating the J-integral.  Incorrectly specifying the radii for the q-function will lead to inaccurate calculation of the J-integral and stress intensity factors.  The q-function should exclude a few elements around the crack tip where $q=1$.  The q-function radii are designated in the `DomainIntegral` block shown in [!ref](input).  This example uses a cutter mesh, shown by the white line in [!ref](stress), to create the initial crack using the `MeshCut2DFractureUserObject` object, also shown in [!ref](input).

!media large_media/xfem/stress_field_label.png
      style=width:80%;margin:auto;padding-top:2.5%;background-color: white;color: black;
      id=stress
      caption= Left: Inclined plate simulation domain showing the stress field in the y-direction.  The mesh dimensions are $W\=H\=40$ and the crack length is $2a\=2$.  The white line in the center is the inclined crack created by the XFEM cutter mesh.  Right: Close up of crack geometry with labels.

!media large_media/xfem/q1_field_label.png
      style=width:70%;margin:auto;padding-top:2.5%;background-color: white;color: black;
      id=qintegral
      caption=Domain integral integration field (q-function) around the bottom crack-tip.  The integration field is integrated over the area between r_inner and r_outer specified in the `DomainIntegral` block in [!ref](input).  The initial XFEM cut created by the `MeshCut2DFractureUserObject` is also shown.

!media large_media/xfem/incline_angle_convergence.png
      style=width:70%;margin:auto;padding-top:2.5%;background-color: white;color: black;
      id=convergence
      caption=Convergence plot for K_I and K_{II} for mesh size ranging from h\=0.2, 0.1, 0.05.  The results shown [!ref](stress) and [!ref](qintegral) are for h\=0.1.

!listing xfem/test/tests/mesh_cut_2D_fracture/inclined_center_crack.i  id=input block=UserObjects DomainIntegral caption=Input file for inclined crack.

The maximum circumferential (hoop) stress criterion is used to determine the crack growth direction [!citet](erdogan1963crack).  This criterion is used in xfem crack growth implementations by [!citet](belytschko_elastic_1999,jiang2020).  For this criterion, the crack will grow in the local crack tip direction $\theta_c$ that maximizes the hoop stress $\sigma_{\theta\theta}$ given in terms of $K_I$ and $K_{II}$ by
\begin{equation}
\label{eq:sigma}
\sigma_{\theta\theta}(r,\theta) =\frac{K_I}{4\sqrt{2\pi r}}\left[3\cos(\theta/2)+\cos(3\theta/2)\right]
-\frac{K_{II}}{4\sqrt{2\pi r}}\left[3\sin(\theta/2)+3\sin(3\theta/2)\right]
\end{equation}

By setting other crack tip stress components to zero to maximize the the stress normal to the crack face, the critical crack tip growth direction is given by
\begin{equation}
\label{eq:theta}
\theta_c=2 \arctan\left(\frac{K_I \pm \sqrt{K_I^2 + 8K_{II}^2}}{4 K_{II}}\right)
\end{equation}
where the $\pm$ is determined by the $\theta_c$ that maximizes $\sigma_{\theta\theta}$.  Simulation and analytical results for the crack tip direction using equation [eq:inclined] are shown in the below figure for the coarsest mesh from [!ref](convergence). The crack growth direction at each end of the inclined crack computed by the `MeshCut2DFractureUserObject` is shown by the '&#9723;' and '$\times$' markers.  These results match the results computed by plugging in $K_{II}$ and $K_I$ from [!ref](convergence) for h=0.2 into [eq:sigma] and [eq:theta] shown by the '&#9679;'.  The simulation results converge to the analytic solution with mesh refinement, as was shown for convergence of $K_{II}$ and $K_I$ in [!ref](convergence).

!media large_media/xfem/crackGrowthDir.png
      style=width:70%;margin:auto;padding-top:2.5%;background-color: white;color: black;
      id=growth
      caption=Crack growth direction from equations [eq:sigma] and [eq:theta] for h\=0.2 from [!ref](convergence).

!bibtex bibliography
