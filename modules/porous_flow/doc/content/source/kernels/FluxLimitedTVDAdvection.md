# FluxLimitedTVDAdvection


This `Kernel` implements the weak form of
\begin{equation}
\nabla\cdot(\mathbf{v}u) \ .
\end{equation}
It employs Kuzmin-Turek [!citep](KuzminTurek2004) stabilization, so needs a corresponding [AdvectiveFluxCalculatorConstantVelocity](AdvectiveFluxCalculatorConstantVelocity.md) UserObject.  This sort of stabilization is described in detail in [A worked example of Kuzmin-Turek stabilization](kt_worked.md).  Also see [numerical diffusion](numerical_diffusion.md) for example of how the Kuzmin-Turek scheme compares with other numerical schemes.

!syntax parameters /Kernels/FluxLimitedTVDAdvection

!syntax inputs /Kernels/FluxLimitedTVDAdvection

!syntax children /Kernels/FluxLimitedTVDAdvection
