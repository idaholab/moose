# ADWallFrictionChengMaterial

The material computes the friction factor using the Cheng-Todreas correlation.

Equations (9.105a), (9.105b), (9.109a), and (9.109b), and tables (9.5a) and (9.5b) from [!cite](todreas2021nuclear) are used.

The Cheng-Todreas correlation gives the friction factor for each subchannel type: interior, edge, or corner. The correlation is a function of the Pitch-to-Diameter ratio, $P/D$, for interior channels, or the Wall-to-Diameter ratio, $W/D$, for edge and corner subchannels. [fig-sub] shows the three subchannel types for square and hexagonal rod bundles, where $P$ is the pitch between the fuel rods, $W$ is the distance between the fuel rod centerline and the bundle wall, and $D$ is the rod diameter.

!media thermal_hydraulics/misc/subchannel_type.png
       style=width:95%;float:center;
       caption=Different subchannel types for a square and hexagonal array rod bundles.
       id=fig-sub

The friction factor is given by:

\begin{equation}
      f_{i} = \frac{C_{i}}{(Re_{i})^n},
\end{equation}

where the subscript $i$ indicates the subchannel type, and the constant $n$ is equal to 1 for laminar flows and 0.18 for tubulent flows. The Reynolds number is calculated as

\begin{equation}
  Re_i = \frac{\rho v D_{h,i}}{\mu},
\end{equation}

where the hydraulic diameter will be a function of the flow area and the wetted perimeter for each subchannel type. The friction factor constant $C_i$ can be obtained from:

\begin{equation}
      C_i = \begin{cases}
      a + b_1\left(\frac{P}{D} - 1\right) + b_2\left(\frac{P}{D} - 1\right)^2 & \text{for interior subchannels}\\
      \\
      a + b_1\left(\frac{W}{D} - 1\right) + b_2\left(\frac{W}{D} - 1\right)^2 & \text{for edge or corner subchannels}\\
    \end{cases}
\end{equation}

The values for the constants $a$, $b_1$, and $b_2$ are given in tables (9.5a) and (9.5b) from [!cite](todreas2021nuclear).

!syntax parameters /Materials/ADWallFrictionChengMaterial

!syntax inputs /Materials/ADWallFrictionChengMaterial

!syntax children /Materials/ADWallFrictionChengMaterial
