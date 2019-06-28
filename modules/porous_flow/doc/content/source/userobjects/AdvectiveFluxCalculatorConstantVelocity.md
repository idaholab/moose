# AdvectiveFluxCalculatorConstantVelocity

The `AdvectiveFluxCalculatorConstantVelocity` computes $K$, $R^{+}$ and $R^{-}$ that are used in the Kuzmin-Turek [!citep](KuzminTurek2004) numerical stabilization scheme, when the velocity is constant and uniform over the mesh.  $K$ is a measure of advective flux between neighbouring nodes, while $R^{+}$ and $R^{-}$ quantify how much antidiffusion to allow around the nodes.  See [A worked example of Kuzmin-Turek stabilization](kt_worked.md) for many explicit details, and [numerical diffusion](numerical_diffusion.md) for example of how the Kuzmin-Turek scheme compares with other numerical schemes.

!syntax parameters /UserObjects/AdvectiveFluxCalculatorConstantVelocity

!syntax inputs /UserObjects/AdvectiveFluxCalculatorConstantVelocity

!syntax children /UserObjects/AdvectiveFluxCalculatorConstantVelocity
