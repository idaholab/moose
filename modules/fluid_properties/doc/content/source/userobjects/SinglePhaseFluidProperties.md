# SinglePhaseFluidProperties

`SinglePhaseFluidProperties` is a base class for all single phase fluid properties.

## Default analytical fluid properties relations

`SinglePhaseFluidProperties` provides a number of default implementations for some fluid properties where
analytical relations hold for all single phase fluid properties. Some of these fluid properties
are also implemented along with their derivatives with regards to the input variables, when
these derivatives can also be analytically described. Relevant automatic differentiation (AD)
implementations are also provided through a `macro` to avoid duplicated code.

The full list of available methods can be found in either the source code or the
[Doxygen](https://mooseframework.inl.gov/docs/doxygen/modules/classSinglePhaseFluidProperties.html) page.

## Variable set conversions

Different fluid applications may require different variable sets, such as (pressure, temperature)
or (specific volume, specific internal energy), depending on the flow regimes of interest and relatedly
the numerical discretization. Fluid properties are not necessarily implemented or known for all variable sets,
so conversions from one variable set to another can be helpful.

For many fluids, analytical closures for these conversions are not known, so `SinglePhaseFluidProperties`
defines several routines for iteratively converting from one variable set
to another. This leverages the [numerical inversion methods utilities](utils/FluidPropertiesUtils.md).
Notably, the following routines are provided:

!listing modules/fluid_properties/include/userobjects/SinglePhaseFluidProperties.h start=p_T_from_v_e end=) const include-end=true

!listing modules/fluid_properties/include/userobjects/SinglePhaseFluidProperties.h start=p_T_from_v_h end=) const include-end=true

!listing modules/fluid_properties/include/userobjects/SinglePhaseFluidProperties.h start=p_T_from_h_s end=) const include-end=true

These routines may then be used to convert from one variable set to another before obtaining the desired
fluid property. For example, this routine converts (pressure, temperature) to (specific volume, specific energy)
to compute entropy.

!listing modules/fluid_properties/src/userobjects/SinglePhaseFluidProperties.C start=SinglePhaseFluidProperties::s_from_p_T end=} include-end=true
