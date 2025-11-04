//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMCutTransitionSubMesh.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMCutTransitionSubMesh);

InputParameters
MFEMCutTransitionSubMesh::validParams()
{
  InputParameters params = MFEMSubMesh::validParams();
  params += MFEMBlockRestrictable::validParams();
  params.addClassDescription(
      "Class to construct an MFEMSubMesh formed from the set of elements that have least one "
      "vertex on the specified cut plane, that lie on one side of the plane, "
      "and that are restricted to the set of user-specified subdomains.");
  params.addRequiredParam<BoundaryName>("cut_boundary",
                                        "The boundary associated with the mesh cut.");
  params.addRequiredParam<BoundaryName>(
      "transition_subdomain_boundary",
      "Name to assign boundary of transition subdomain not shared with cut surface.");
  params.addRequiredParam<SubdomainName>(
      "transition_subdomain",
      "The name of the subdomain to be created on the mesh comprised of the set of elements "
      "adjacent to the cut surface on one side.");
  params.addRequiredParam<SubdomainName>(
      "closed_subdomain",
      "The name of the subdomain attribute to be created comprised of the set of all elements "
      "of the closed geometry, including the new transition region.");
  return params;
}

MFEMCutTransitionSubMesh::MFEMCutTransitionSubMesh(const InputParameters & parameters)
  : MFEMSubMesh(parameters),
    MFEMBlockRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh()),
    _cut_boundary(getParam<BoundaryName>("cut_boundary")),
    _cut_submesh(std::make_shared<mfem::ParSubMesh>(mfem::ParSubMesh::CreateFromBoundary(
        getMFEMProblem().mesh().getMFEMParMesh(),
        getMesh().bdr_attribute_sets.GetAttributeSet(_cut_boundary)))),
    _transition_subdomain_boundary(getParam<BoundaryName>("transition_subdomain_boundary")),
    _transition_subdomain(getParam<SubdomainName>("transition_subdomain")),
    _closed_subdomain(getParam<SubdomainName>("closed_subdomain")),
    _cut_normal(3)
{
}

void
MFEMCutTransitionSubMesh::buildSubMesh()
{
  labelMesh(getMFEMProblem().mesh().getMFEMParMesh());
  _submesh = std::make_shared<mfem::ParSubMesh>(mfem::ParSubMesh::CreateFromDomain(
      getMesh(), getMesh().attribute_sets.GetAttributeSet(_transition_subdomain)));
  _submesh->attribute_sets.attr_sets = getMesh().attribute_sets.attr_sets;
  _submesh->bdr_attribute_sets.attr_sets = getMesh().bdr_attribute_sets.attr_sets;
}

void
MFEMCutTransitionSubMesh::labelMesh(mfem::ParMesh & parent_mesh)
{
  int mpi_comm_rank = getMFEMProblem().getProblemData().myid;
  int mpi_comm_size = getMFEMProblem().getProblemData().num_procs;

  // First determine face normal based on the first boundary element found on the cut
  // to use when determining orientation relative to the cut
  const mfem::Array<int> & parent_cut_element_id_map = _cut_submesh->GetParentElementIDMap();
  int rank_with_submesh = -1;
  if (parent_cut_element_id_map.Size() > 0)
  {
    int reference_face = parent_cut_element_id_map[0];
    _cut_normal = findFaceNormal(parent_mesh, parent_mesh.GetBdrElementFaceIndex(reference_face));
    rank_with_submesh = mpi_comm_rank;
  }
  MPI_Allreduce(MPI_IN_PLACE, &rank_with_submesh, 1, MPI_INT, MPI_MAX, getMFEMProblem().getComm());
  MPI_Bcast(_cut_normal.GetData(),
            _cut_normal.Size(),
            MFEM_MPI_REAL_T,
            rank_with_submesh,
            getMFEMProblem().getComm());
  // Iterate over all vertices on cut, find elements with those vertices,
  // and declare them transition elements if they are on the +ve side of the cut
  mfem::Array<int> transition_els;
  std::vector<HYPRE_BigInt> global_cut_vert_ids;
  mfem::Array<HYPRE_BigInt> gi;
  parent_mesh.GetGlobalVertexIndices(gi);
  std::unique_ptr<mfem::Table> vert_to_elem(parent_mesh.GetVertexToElementTable());
  const mfem::Array<int> & cut_to_parent_vertex_id_map = _cut_submesh->GetParentVertexIDMap();
  for (int i = 0; i < _cut_submesh->GetNV(); ++i)
  {
    int cut_vert = cut_to_parent_vertex_id_map[i];
    global_cut_vert_ids.push_back(gi[cut_vert]);
    int ne = vert_to_elem->RowSize(cut_vert); // number of elements touching cut vertex
    const int * els_adj_to_cut = vert_to_elem->GetRow(cut_vert); // elements touching cut vertex
    for (int i = 0; i < ne; i++)
    {
      const int el_adj_to_cut = els_adj_to_cut[i];
      if (isInDomain(el_adj_to_cut, getSubdomainAttributes(), parent_mesh) &&
          isPositiveSideOfCut(el_adj_to_cut, cut_vert, parent_mesh))
        transition_els.Append(el_adj_to_cut);
    }
  }

  // share cut verts coords or global ids across all procs
  int n_cut_vertices = global_cut_vert_ids.size();
  std::vector<int> cut_vert_sizes(mpi_comm_size, 0);
  MPI_Allgather(
      &n_cut_vertices, 1, MPI_INT, &cut_vert_sizes[0], 1, MPI_INT, getMFEMProblem().getComm());
  // Make an offset array and total the sizes.
  std::vector<int> n_vert_offset(mpi_comm_size, 0);
  for (int i = 1; i < mpi_comm_size; i++)
    n_vert_offset[i] = n_vert_offset[i - 1] + cut_vert_sizes[i - 1];
  int global_n_cut_vertices = 0;
  for (int i = 0; i < mpi_comm_size; i++)
    global_n_cut_vertices += cut_vert_sizes[i];

  // Gather the queries to all ranks.
  std::vector<HYPRE_BigInt> all_cut_verts(global_n_cut_vertices, 0);
  MPI_Allgatherv(&global_cut_vert_ids[0],
                 n_cut_vertices,
                 HYPRE_MPI_BIG_INT,
                 &all_cut_verts[0],
                 &cut_vert_sizes[0],
                 &n_vert_offset[0],
                 HYPRE_MPI_BIG_INT,
                 getMFEMProblem().getComm());

  // Detect shared vertices and add corresponding elements
  for (int g = 1, sv = 0; g < parent_mesh.GetNGroups(); g++)
  {
    for (int gv = 0; gv < parent_mesh.GroupNVertices(g); gv++, sv++)
    {
      // all elements touching this shared vertex should be updated
      int cut_vert = parent_mesh.GroupVertex(g, gv);
      for (std::size_t i = 0; i < all_cut_verts.size(); i += 1)
      {
        if (gi[cut_vert] == all_cut_verts[i]) // check if shared vertex is on the cut plane
        {
          int ne = vert_to_elem->RowSize(cut_vert); // number of elements touching cut vertex
          const int * els_adj_to_cut =
              vert_to_elem->GetRow(cut_vert); // elements touching cut vertex
          for (int i = 0; i < ne; i++)
          {
            const int el_adj_to_cut = els_adj_to_cut[i];
            if (isInDomain(el_adj_to_cut, getSubdomainAttributes(), parent_mesh) &&
                isPositiveSideOfCut(el_adj_to_cut, cut_vert, parent_mesh))
              transition_els.Append(el_adj_to_cut);
          }
        }
      }
    }
  }

  transition_els.Sort();
  transition_els.Unique();

  setAttributes(parent_mesh, transition_els);
}

mfem::Vector
MFEMCutTransitionSubMesh::findFaceNormal(const mfem::ParMesh & mesh, const int & face)
{
  mooseAssert(mesh.SpaceDimension() == 3,
              "MFEMCutTransitionSubMesh only works in 3-dimensional meshes!");
  mfem::Vector normal;
  mfem::Array<int> face_verts;
  std::vector<mfem::Vector> v;
  mesh.GetFaceVertices(face, face_verts);

  // First we get the coordinates of 3 vertices on the face
  for (auto vtx : face_verts)
  {
    mfem::Vector vtx_coords(3);
    for (int j = 0; j < 3; ++j)
      vtx_coords[j] = mesh.GetVertex(vtx)[j];
    v.push_back(vtx_coords);
  }

  // Now we find the unit vector normal to the face
  v[0] -= v[1];
  v[1] -= v[2];
  v[0].cross3D(v[1], normal);
  normal /= normal.Norml2();
  return normal;
}

bool
MFEMCutTransitionSubMesh::isPositiveSideOfCut(const int & el,
                                              const int & el_vertex_on_cut,
                                              mfem::ParMesh & parent_mesh)
{
  const int sdim = parent_mesh.SpaceDimension();
  mfem::Vector el_center(3);
  parent_mesh.GetElementCenter(el, el_center);
  mfem::Vector vertex_coords(parent_mesh.GetVertex(el_vertex_on_cut), sdim);
  mfem::Vector relative_center(sdim);
  for (int j = 0; j < sdim; j++)
    relative_center[j] = el_center[j] - vertex_coords[j];
  return _cut_normal * relative_center > 0;
}

void
MFEMCutTransitionSubMesh::setAttributes(mfem::ParMesh & parent_mesh,
                                        mfem::Array<int> & transition_els)
{
  // Generate a set of local new attributes for transition region elements
  const int old_max_attr = parent_mesh.attributes.Max();
  mfem::Array<int> new_attrs(old_max_attr);
  new_attrs = -1;
  for (auto const & transition_el : transition_els)
  {
    const int old_attr = parent_mesh.GetAttribute(transition_el);
    new_attrs[old_attr - 1] = old_max_attr + old_attr;
    // Set the new attribute IDs for transition region elements
    parent_mesh.SetAttribute(transition_el, new_attrs[old_attr - 1]);
  }

  // Distribute local attribute IDs to other MPI ranks to create set of new
  // global attribute IDs for the transition region.
  MPI_Allreduce(
      MPI_IN_PLACE, new_attrs, old_max_attr, MPI_INT, MPI_MAX, getMFEMProblem().getComm());

  mfem::AttributeSets & attr_sets = parent_mesh.attribute_sets;
  mfem::AttributeSets & bdr_attr_sets = parent_mesh.bdr_attribute_sets;
  // Create attribute set labelling the newly created transition region on one side of the cut
  attr_sets.CreateAttributeSet(_transition_subdomain);
  // Create attribute set labelling the entire closed geometry
  attr_sets.SetAttributeSet(_closed_subdomain, getSubdomainAttributes());
  // Add the new domain attributes to new attribute sets
  const std::set<std::string> attr_set_names = attr_sets.GetAttributeSetNames();
  for (int old_attr = 1; old_attr <= old_max_attr; ++old_attr)
  {
    int new_attr = new_attrs[old_attr - 1];
    // Add attributes only if they're transition region attributes
    if (new_attr != -1)
    {
      attr_sets.AddToAttributeSet(_transition_subdomain, new_attr);
      for (auto const & attr_set_name : attr_set_names)
      {
        const mfem::Array<int> & attr_set = attr_sets.GetAttributeSet(attr_set_name);
        // If attribute set has the old attribute of the transition region, add the new one
        if (attr_set.Find(old_attr) != -1)
          attr_sets.AddToAttributeSet(attr_set_name, new_attr);
      }
    }
  }

  // Create boundary attribute set labelling the exterior of the newly created
  // transition region, excluding the cut
  bdr_attr_sets.SetAttributeSet(_transition_subdomain_boundary,
                                mfem::Array<int>({parent_mesh.bdr_attributes.Max() + 1}));

  parent_mesh.SetAttributes();
}

bool
MFEMCutTransitionSubMesh::isInDomain(const int & element,
                                     const mfem::Array<int> & subdomains,
                                     const mfem::ParMesh & mesh)
{
  // element<0 for ghost elements
  if (element < 0)
    return true;

  for (const auto & subdomain : subdomains)
    if (mesh.GetAttribute(element) == subdomain)
      return true;
  return false;
}

#endif
