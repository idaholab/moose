# LAROMANCE6DInterpolation

!if! function=hasCapability('neml2')

!alert note
This is a NEML2 model. See the [NEML2 syntax documentation](syntax/NEML2/index.md) for guidance on using NEML2 models in a MOOSE simulation.

The documentation is for a NEML2 [!citep](neml2osti) implementation of the LAROMance surrogate model.  The LAROMance surrogate model computes the inelastic effective strain rate used in an isotropic viscoplastic constitutive model update [!cite](tallman-2020).  The LAROMance surrogate model is fit to a database of polycrystalline crystal plasticity simulations.  The material's microstructure, as described by the distribution of dislocation content, temperature, stress, and environment factor (precipitate phase fraction, neutron fluence, etc. ), varied in the crystal plasticity creep simulations used to create the database.  The inelastic strain history and mobile and immobile dislocation evolution from the crystal plasticity simulations are then fit to an element-based LAROMance model [!citep](ruybalid_osti24, munday_osti25). The element-based input space provides an interpolation grid for the output state of the material.  This version of the LAROMance model uses piecewise-continuous linear functions to interpolate the input grid.  The six-dimensional (6D) input space for the LAROMance model, $f\left(T,\epsilon_{p},\sigma_{vm},\rho_c,\rho_w,\Phi_{mx}\right)$, corresponds to the variables varied in the LApx simulations---that is, temperature ($T$ in units K), von Mises stress ($\sigma_{vm}$ in units MPa), accumulated effective inelastic strain ($\epsilon_{p}$), mobile or cell dislocation density ($\rho_c$ in units $m^{-2}$), immobile or wall dislocation density ($\rho_w$ in units $m^{-2}$), and environmental factor ($\Phi_{mx}$ in units of number density; $count/m^{-3}$).  Interpolated outputs from the LAROMance model are inelastic effective strain rate ($\dot{\epsilon}_{p}$), and mobile and immobile dislocation density evolution rates ($\dot{\rho}_c$ and  $\dot{\rho}_w$).  Transforms are applied to the input grid and output state to normalize the interpolation space, apply constraints on the output, and allow linear regression to fit a nonlinear relationship between inputs and outputs such as the logarithmic evolution of dislocation density due to accumulated effective inelastic strain [!cite](tallman-2020).  Additional information about the LAROMance models and a previous implementation is available [here](/LAROMANCE.md).

The LAROMance model implementation in NEML2 is based on multilinear interpolation on a regular nonuniform grid.  The six dimensional interpolation grid contains $\left(T,\epsilon_{p},\sigma_{vm},\rho_c,\rho_w,\Phi_{mx}\right)$, nodal output values $\left(\dot{\epsilon}_{p},\dot{\rho}_c,\dot{\rho}_w\right)$, and transforms are contained in the JSON database file.  Extrapolation is performed for $\left(T,\epsilon_{p},\sigma_{vm},\rho_c,\rho_w,\Phi_{mx}\right)$ outside the grid domain.  See report for verification and additional details [!cite](munday_osti25).

Additional information about a previous version of the LAROMANCE model are documented in [the LAROMANCE documentation](/LAROMANCE.md).

## Example Input File

An example input for a NEML2 LAROMANCE model is shown below for inelastic plastic strain rate interpolation, `model_file_variable_name = 'out_ep'`.  The json file contains `out_ep` on a 6D grid defined by the variables (`von_mises_stress`, `equivalent_plastic_strain`, `cell_dislocation_density`, `wall_dislocation_density`, `temperature`, `env_factor`) which correspond to $\left(T,\epsilon_{p},\sigma_{vm},\rho_c,\rho_w,\Phi_{mx}\right)$.  The json file only contains interpolation grids for `out_ep`, `out_wall`, and `out_cell` which correspond to $\left(\dot{\epsilon}_{p},\dot{\rho}_c,\dot{\rho}_w\right)$.  In the below input file, NEML2 performs a nonlinear solve for $\dot{\epsilon}_{p}$, `output_rate = 'state/ep_rate'`, which uses automatic differentiation to compute the derivative of the constitutive model with respect to `out_ep`, $\partial f/\partial\epsilon_{p}$.  The cell and wall dislocation density rates (`out_wall` and `out_cell`) are interpolated in seperate material blocks in the below input file.  The dislocation densities are computed from the rates using forward Euler time integration and derivatives for `out_wall` and `out_cell` are never computed.  The dislocation densities are not included in the nonlinear material solve and are lagged by a timestep.

!listing test/tests/neml2/laromance/models/laromance_matl_radial_return.i block=Models/rom_ep

!bibtex bibliography

!if-end!

!else

!include neml2/neml2_warning.md
