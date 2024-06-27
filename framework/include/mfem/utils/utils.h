#pragma once
#include "../common/pfem_extras.hpp"
#include "coefficients.h"
#include "helmholtz_projector.h"
#include "hephaestus_solvers.h"
#include "inputs.h"

namespace platypus
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
void CleanDivergence(mfem::ParGridFunction & Vec_GF, platypus::InputParameters solve_pars);

// Uses the HelmholtzProjector auxsolver to return a divergence-free GridFunction. This version of
// the function allows the user to set up boundary conditions for the HelmholtzProjector.
void CleanDivergence(platypus::GridFunctions & gfs,
                     platypus::BCMap & bcs,
                     const std::string vec_gf_name,
                     const std::string scalar_gf_name,
                     platypus::InputParameters solve_pars);

} // namespace platypus