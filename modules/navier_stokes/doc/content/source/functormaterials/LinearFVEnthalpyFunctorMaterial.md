# LinearFVEnthalpyFunctorMaterial

This [FunctorMaterial](syntax/FunctorMaterials/index.md) class is needed when the user wants to solve for the enthalpy conservation for the linear finite-volume implementation of the incompressible/weakly-compressible Navier-Stokes equations.

This material class defines the functors to convert from specific enthalpy and pressure to temperature ('T_from_p_h') and to convert from temperature and pressure to specific enthalpy ('h_from_p_T').

When solving the enthalpy conservation equation and obtaining the specific enthalpy ($h$) as the solution, two functors are required.
The 'T_from_p_h' functor computes the temperature field, which is then used to evaluate temperature-dependent thermophysical properties. The 'h_from_p_T' functor is primarily utilized for setting boundary conditions where the temperature is specified.

The specific enthalpy is defined as:

\begin{equation}
  h(T) = \int_{T_{ref}}^{T} c_p(p, T')dT' + h_{ref},
\end{equation}

where

- $c_p(p, T)$ is the pressure and temperature-dependent specific heat
- $T_{ref}$ is a reference temperature where the reference enthalpy $h_{ref}$ is defined

## Example input syntax 1: Fluid Properties

In this input, a `LinearFVEnthalpyFunctorMaterial` is defined to solve for a 1D heated channel using lead fluid properties with a fixed mass flux.

The `FluidProperties` object for lead is defined in

!listing test/tests/finite_volume/wcns/enthalpy_equation/1d_test_h_fp.i block=FluidProperties

Then, the `LinearFVEnthalpyFunctorMaterial` is defined taking the `FluidProperties` object as an input, which already contains the 'h_from_p_T' and 'T_from_p_h' functors defined.

!listing test/tests/finite_volume/wcns/enthalpy_equation/1d_test_h_fp.i block=FunctorMaterials/enthalpy_material

## Example input syntax 2: User-Defined Properties

In this input, a `LinearFVEnthalpyFunctorMaterial` is defined to solve for a 1D heated channel using FLiNaK properties with a fixed mass flux, defined by the user.

!listing test/tests/finite_volume/wcns/enthalpy_equation/1d_test_h.i block=FunctorMaterials/enthalpy_material

This functor material takes the user-defined 'h_from_p_T' and 'T_from_p_h' functors as inputs. The functors are defined in

!listing test/tests/finite_volume/wcns/enthalpy_equation/1d_test_h.i block=FunctorMaterials/h_from_p_T_functor

!listing test/tests/finite_volume/wcns/enthalpy_equation/1d_test_h.i block=FunctorMaterials/T_from_p_h_functor

for the linear specific heat:

!listing test/tests/finite_volume/wcns/enthalpy_equation/1d_test_h.i block=FunctorMaterials/cp

!alert warning
If using user-defined properties, it is the responsibility of the user to update these properties often enough. For example auxiliary variable properties would only be updated at the end of every time step by default, which would introduce a lag. 

!syntax parameters /FunctorMaterials/LinearFVEnthalpyFunctorMaterial

!syntax inputs /FunctorMaterials/LinearFVEnthalpyFunctorMaterial

!syntax children /FunctorMaterials/LinearFVEnthalpyFunctorMaterial
