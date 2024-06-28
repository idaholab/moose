#pragma once
#include "../common/pfem_extras.hpp"
#include "coefficients.h"
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

} // namespace platypus