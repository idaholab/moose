
# BarrierFunctionMaterial
!description /Materials/BarrierFunctionMaterial

Two phase free energy phase transformation barrier term.
With the `g_order` parameter set to `SIMPLE` the function is defined as

$$
g(\eta) = \eta^2(1-\eta)^2
$$

and with the  `g_order` parameter set to `LOW` it is defined as

$$
g(\eta) = \eta(1-\eta)
$$


!parameters /Materials/BarrierFunctionMaterial

!inputfiles /Materials/BarrierFunctionMaterial

!childobjects /Materials/BarrierFunctionMaterial
