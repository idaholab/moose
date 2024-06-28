#include "utils.h"

namespace platypus
{

template <typename T>
void
IfDelete(T * ptr)
{
  if (ptr != nullptr)
    delete ptr;
}

void
InheritBdrAttributes(const mfem::ParMesh * parent_mesh, mfem::ParSubMesh * child_mesh)
{

  int face, ori, att;
  auto map = child_mesh->GetParentToSubMeshFaceIDMap();

  for (int bdr = 0; bdr < parent_mesh->GetNBE(); ++bdr)
  {

    parent_mesh->GetBdrElementFace(bdr, &face, &ori);
    if (map[face] != -1)
    {
      att = parent_mesh->GetBdrAttribute(bdr);
      auto * new_elem = child_mesh->GetFace(map[face])->Duplicate(child_mesh);
      new_elem->SetAttribute(att);
      child_mesh->AddBdrElement(new_elem);
    }
  }

  child_mesh->FinalizeTopology();
  child_mesh->Finalize();
  child_mesh->SetAttributes();
}

void
AttrToMarker(const mfem::Array<int> attr_list, mfem::Array<int> & marker_list, int max_attr)
{

  marker_list.SetSize(max_attr);
  marker_list = 0;

  for (auto a : attr_list)
    marker_list[a - 1] = 1;
}
} // namespace platypus