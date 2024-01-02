# ADWallHTCGnielinskiAnnularMaterial

The material computes the convective heat transfer coefficient using the Gnielinski correlation
for turbulent flow in annular ducts [!citep](gnielinski2010).

The Nusselt number is calculated as follows:

!equation
\text{Nu} = \frac{(f_\text{ann}/8) \text{Re} \text{Pr}}{k_1 + 12.7\sqrt{f_\text{ann}/8}(\text{Pr}^{2/3} - 1)}
  \left[1 + \left(\frac{D_h}{L}\right)^{2/3}\right] F_\text{ann} K \,,

!equation
k_1 = 1.07 + \frac{900}{\text{Re}} - \frac{0.63}{1 + 10\text{Pr}} \,,

!equation
f_\text{ann} = (1.8 \log_{10}\text{Re}^* - 1.5)^{-2} \,,

!equation
\text{Re}^* = \text{Re} \frac{(1 + a^2)\ln a + (1 - a^2)}{(1 - a)^2 \ln a} \,,

!equation
K = \left\{\begin{array}{l l}
  \left(\frac{T}{T_w}\right)^n & \text{Fluid is gas} \\
  \left(\frac{\text{Pr}}{\text{Pr}_w}\right)^{0.11} & \text{Fluid is liquid}
  \end{array}\right.  \,,

!equation
F_\text{ann} = \left\{\begin{array}{l l}
  0.75 a^{-0.17} & \text{Heat transfer at inner wall} \\
  0.9 - 0.15 a^{0.6} & \text{Heat transfer at outer wall}
  \end{array}\right. \,,

!equation
D_h = D_o - D_i \,,

!equation
a = \frac{D_i}{D_o} \,,

where:

- $\text{Re}$ is the Reynolds number,
- $\text{Pr}$ is the Prandtl number,
- $\text{Pr}_w$ is the Prandtl number obtained by evaluating properties at the wall temperature,
- $L$ is the channel length,
- $T$ is the fluid temperature,
- $T_w$ is the wall temperature, and
- $n$ is the gas exponent, which varies by gas and situation.

Lastly, the heat transfer coefficient is calculated as

!equation
h = \frac{\text{Nu} k}{D_h} \,.

!syntax parameters /Materials/ADWallHTCGnielinskiAnnularMaterial

!syntax inputs /Materials/ADWallHTCGnielinskiAnnularMaterial

!syntax children /Materials/ADWallHTCGnielinskiAnnularMaterial
