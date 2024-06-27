#pragma once
#include "../common/pfem_extras.hpp"
#include "coefficients.hpp"
#include "helmholtz_projector.hpp"
#include "hephaestus_solvers.hpp"
#include "inputs.hpp"

namespace hephaestus
{

// Useful functions available to all classes

// Deletes a pointer if it's not null
template <typename T>
void IfDelete(T * ptr);

// This is a hotfix for the MFEM issue where internal boundary attributes are not passed down from
// parent to child submesh. Once this issue is resolved on the MFEM side, this function will be
// deprecated.
void InheritBdrAttributes(const mfem::ParMesh * parent_mesh, mfem::ParSubMesh * child_mesh);

// Takes in an array of attributes and turns into a marker array.
void AttrToMarker(const mfem::Array<int> attr_list, mfem::Array<int> & marker_list, int max_attr);

// Uses the HelmholtzProjector auxsolver to return a divergence-free GridFunction. This version of
// the function assumes all natural boundary conditions for the HelmholtzProjector equal zero.
void CleanDivergence(mfem::ParGridFunction & Vec_GF, hephaestus::InputParameters solve_pars);

// Uses the HelmholtzProjector auxsolver to return a divergence-free GridFunction. This version of
// the function allows the user to set up boundary conditions for the HelmholtzProjector.
void CleanDivergence(hephaestus::GridFunctions & gfs,
                     hephaestus::BCMap & bcs,
                     const std::string vec_gf_name,
                     const std::string scalar_gf_name,
                     hephaestus::InputParameters solve_pars);

} // namespace hephaestus