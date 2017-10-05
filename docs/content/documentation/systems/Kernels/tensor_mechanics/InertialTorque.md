# InertialTorque
!syntax description /Kernels/InertialTorque


##Description

This kernel computes the $i^{\mathrm{th}}$ component of inertial torque, viz
$$
\rho \epsilon_{ijk}u_{j}\ddot{u}_{k} \ .
$$
In this equation $\rho$ is the material's density (a Material Property), $\epsilon_{ijk}$ is the permutation pseudotensor (the Levi-Cevita tensor), $u_{j}$ is the $j^{\mathrm{th}}$ component of displacement, and $\ddot{u}_{k}$ is the $k^{\mathrm{th}}$ component of the acceleration.

This Kernel is used in dynamic Cosserat solid-mechanics problems.

Newmark time integration is used, which means the Newmark acceleration is defined by
$$
\ddot{u}_{\mathrm{Newmark}} = \frac{1}{\beta} \left( \frac{u - u_{\mathrm{old}}}{{\mathrm{d}t}^{2}} - \frac{\dot{u}_{\mathrm{old}}}{{\mathrm{d}t}} - (\tfrac{1}{2} - \beta)\ddot{u}_{\mathrm{old}} \right) \ ,
$$
and the velocity by
$$
\dot{u} = \dot{u}_{\mathrm{old}} + {\mathrm{d}t} (1 - \gamma)\ddot{u}_{\mathrm{old}} + \gamma {\mathrm{d}t}\ddot{u}_{\mathrm{Newmark}} \ .
$$
The Newmark parameters must satisfy $0\leq \gamma\leq 1$ and $0 \leq \beta \leq 1/2$.  When $\gamma\geq 1/2$ and $\beta\geq \tfrac{1}{4}(\gamma + \tfrac{1}{2})^{2}$, the Newmark scheme is unconditionally stable.

Parameters ($\eta$ and $\alpha$) are also included that allow Rayleigh damping and HHT time integration, meaning that the final form for acceleration is
$$
\ddot{u} = \ddot{u}_{\mathrm{Newmark}} + \eta\left((1 + \alpha)\dot{u}_{\mathrm{Newmark}} - \alpha\dot{u}_{\mathrm{old}}\right) \ .
$$
If damping is utilised (ie, if $\eta\neq 0$) then the "Dynamic" versions of the StressDivergenceTensors Kernels should be used.

!syntax parameters /Kernels/InertialTorque

!syntax inputs /Kernels/InertialTorque

!syntax children /Kernels/InertialTorque
