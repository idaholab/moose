# ADWallFrictionColebrookWhiteMaterial

!syntax description /Materials/ADWallFrictionColebrookWhiteMaterial

The Colebrook-White friction factor $f$ is computed using

!equation
\dfrac{1}{\sqrt{f}}= -2 \log \left( \dfrac { \varepsilon} {3.7 D_h} + \dfrac {2.51} {\mathrm{Re} \sqrt{f}} \right),

where $Re$ is the Reynolds number, $D_h$ is the hydraulic diameter and $\varepsilon$ is the roughness. This equation is implicit and is solved iteratively to a relative precision of $10^{-14}$. The Colebrook-White correlation is only valid for $Re>4000$. A warning is issued if the calculated value is below this.

!syntax parameters /Materials/ADWallFrictionColebrookWhiteMaterial

!syntax inputs /Materials/ADWallFrictionColebrookWhiteMaterial

!syntax children /Materials/ADWallFrictionColebrookWhiteMaterial
