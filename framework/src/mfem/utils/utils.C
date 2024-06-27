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

void
CleanDivergence(std::shared_ptr<mfem::ParGridFunction> Vec_GF, platypus::InputParameters solve_pars)
{

  platypus::InputParameters pars;
  platypus::GridFunctions gfs;
  platypus::FESpaces fes;
  platypus::BCMap bcs;

  gfs.Register("Vector_GF", Vec_GF);
  pars.SetParam("VectorGridFunctionName", std::string("Vector_GF"));
  pars.SetParam("SolverOptions", solve_pars);
  platypus::HelmholtzProjector projector(pars);
  projector.Project(gfs, fes, bcs);
}

void
CleanDivergence(platypus::GridFunctions & gfs,
                platypus::BCMap & bcs,
                const std::string vec_gf_name,
                const std::string scalar_gf_name,
                platypus::InputParameters solve_pars)
{

  platypus::InputParameters pars;
  platypus::FESpaces fes;

  pars.SetParam("VectorGridFunctionName", vec_gf_name);
  pars.SetParam("ScalarGridFunctionName", scalar_gf_name);
  pars.SetParam("SolverOptions", solve_pars);
  platypus::HelmholtzProjector projector(pars);
  projector.Project(gfs, fes, bcs);
}

} // namespace platypus