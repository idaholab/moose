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
  params.addClassDescription("Class to construct an MFEMSubMesh formed from the subspace of the "
                             "parent mesh restricted to the set of user-specified subdomains.");
  params.addRequiredParam<BoundaryName>(
      "cut_boundary",
      "The boundary associated with the mesh cut.");
  params.addRequiredParam<BoundaryName>(
      "transition_subdomain_boundary",
      "Name to assign boundary of transition subdomain not shared with cut surface.");      
  params.addParam<SubdomainName>(
      "transition_subdomain",
      "cut",
      "The name of the subdomain to be created on the mesh comprised of the set of elements "
      "adjacent to the cut surface on one side.");
  params.addParam<SubdomainName>(
      "closed_subdomain",
      "closed_subdomain",
      "The name of the subdomain attribute to be created comprised of the set of all elements "
      "of the closed goemetry, including the new transition region.");      
  return params;
}

MFEMCutTransitionSubMesh::MFEMCutTransitionSubMesh(const InputParameters & parameters)
  : MFEMSubMesh(parameters),
    MFEMBlockRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh()),
    _cut_boundary(getParam<BoundaryName>("cut_boundary")),
    _cut_bdr_attribute(std::stoi(_cut_boundary)),
    _cut_submesh(std::make_shared<mfem::ParSubMesh>(mfem::ParSubMesh::CreateFromBoundary(
        getMFEMProblem().mesh().getMFEMParMesh(), mfem::Array<int>({_cut_bdr_attribute})))),
    _transition_subdomain_boundary(getParam<BoundaryName>("transition_subdomain_boundary")),
    _transition_subdomain(getParam<SubdomainName>("transition_subdomain")),
    _closed_subdomain(getParam<SubdomainName>("closed_subdomain")),
    _subdomain_label(getMFEMProblem().mesh().getMFEMParMesh().attributes.Max() + 1)
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
  /// First create a plane based on the first boundary element found on the cut
  /// to use when determining orientation relative to the cut
  Plane3D plane;
  const mfem::Array<int> & parent_cut_element_id_map = _cut_submesh->GetParentElementIDMap();
  if (parent_cut_element_id_map.Size() > 0)
  {
    int reference_face = parent_cut_element_id_map[0];
    plane.make3DPlane(parent_mesh, parent_mesh.GetBdrElementFaceIndex(reference_face));
  }
  mfem::Vector orientation({0., 1., 0.});
  // Plane may not exist on all processors!!

  /// Iterate over all vertices on cut, find elements with those vertices,
  /// and declare them transition elements if they are on the +ve side of the cut
  mfem::Array<int> transition_els;
  std::vector<int> global_cut_vert_ids;
  mfem::Array<HYPRE_BigInt> gi;
  parent_mesh.GetGlobalVertexIndices(gi);
  mfem::Table *vert_to_elem  = parent_mesh.GetVertexToElementTable();
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
      mfem::Vector el_center(3);
      parent_mesh.GetElementCenter(el_adj_to_cut, el_center);
      if (isInDomain(el_adj_to_cut, getSubdomainAttributes(), parent_mesh) &&
          plane.side(el_center) == 1)
        transition_els.Append(el_adj_to_cut);
    }
  }

  // share cut verts coords or global ids across all procs
  int mpi_comm_size = 1;
  MPI_Comm_size(getMFEMProblem().mesh().getMFEMParMesh().GetComm(), &mpi_comm_size);
  int n_cut_vertices = global_cut_vert_ids.size();
  std::vector<int> cut_vert_sizes(mpi_comm_size, 0);
  MPI_Allgather(&n_cut_vertices,
                1,
                MPI_INT,
                &cut_vert_sizes[0],
                1,
                MPI_INT,
                getMFEMProblem().mesh().getMFEMParMesh().GetComm());
  // Make an offset array and total the sizes.
  std::vector<int> n_vert_offset(mpi_comm_size, 0);
  for (int i = 1; i < mpi_comm_size; i++)
    n_vert_offset[i] = n_vert_offset[i - 1] + cut_vert_sizes[i - 1];
  int global_n_cut_vertices = 0;
  for (int i = 0; i < mpi_comm_size; i++)
    global_n_cut_vertices += cut_vert_sizes[i];

  // Gather the queries to all ranks.
  std::vector<int> all_cut_verts(global_n_cut_vertices, 0);
  MPI_Allgatherv(&global_cut_vert_ids[0],
                 n_cut_vertices,
                 MPI_INT,
                 &all_cut_verts[0],
                 &cut_vert_sizes[0],
                 &n_vert_offset[0],
                 MPI_INT,
                 getMFEMProblem().mesh().getMFEMParMesh().GetComm());

  // sign of dot product of (element centre - vert) with direction vec gives ori
  // if shared vert coord = cut vert coord then shared vert touches
  // create container storing global ID for all shared verts

  // for all verts in cut submesh,
  // if global vert

  /// Detect shared vertices and add corresponding elements
  for (int g = 1, sv = 0; g < parent_mesh.GetNGroups(); g++)
  {
    for (int gv = 0; gv < parent_mesh.GroupNVertices(g); gv++, sv++)
    {
      // all els touching this shared vertex plvtx should be updated
      int cut_vert = parent_mesh.GroupVertex(g, gv);
      for (size_t i = 0; i < all_cut_verts.size(); i += 1)
      {
        if (gi[cut_vert] == all_cut_verts[i]) // check if shared vertex is on the cut plane
        {
          int ne = vert_to_elem->RowSize(cut_vert); // number of elements touching cut vertex
          const int * els_adj_to_cut =
              vert_to_elem->GetRow(cut_vert); // elements touching cut vertex
          for (int i = 0; i < ne; i++)
          {
            const int el_adj_to_cut = els_adj_to_cut[i];
            mfem::Vector el_center(3);
            parent_mesh.GetElementCenter(el_adj_to_cut, el_center);
            const int sdim = parent_mesh.SpaceDimension();
            mfem::Vector coord(parent_mesh.GetVertex(cut_vert), sdim);
            mfem::Vector relative_center(sdim);
            for (int j = 0; j < sdim; j++)
            {
              relative_center[j] = el_center[j] - coord[j];
            }
            double check = orientation * relative_center;
            if (isInDomain(el_adj_to_cut, getSubdomainAttributes(), parent_mesh) && check > 0)
              transition_els.Append(el_adj_to_cut);
          }
        }
      }
    }
  }

  delete vert_to_elem;
  transition_els.Sort();
  transition_els.Unique();

  setAttributes(parent_mesh, transition_els);
}

void
MFEMCutTransitionSubMesh::setAttributes(mfem::ParMesh & parent_mesh,
                                        mfem::Array<int> & transition_els)
{
  /// Generate a set of local new attributes for transition region elements
  const int old_max_attr = parent_mesh.attributes.Max();
  mfem::Array<int> new_attrs(old_max_attr);
  for (int i = 0; i < new_attrs.Size(); ++i)
    new_attrs[i] = i + 1;
  for (int i = 0; i < transition_els.Size(); ++i)
  {
    const auto & transition_el = transition_els[i];
    const int old_attr = parent_mesh.GetAttribute(transition_el);
    new_attrs[old_attr-1] = old_max_attr + old_attr%old_max_attr;
  }

  /// Distribute local attribute IDs to other MPI ranks to create set of new
  /// global attribute IDs for the transition region.
  mfem::Array<int> global_new_attrs(old_max_attr);
  global_new_attrs = -1;
  MPI_Allreduce(new_attrs, global_new_attrs, old_max_attr,
              MPI_INT, MPI_MAX, getMFEMProblem().mesh().getMFEMParMesh().GetComm());

  /// Set the new attribute IDs for transition region elements
  for (int i = 0; i < transition_els.Size(); ++i)
  {
    const auto & transition_el = transition_els[i];
    const int old_attr = parent_mesh.GetAttribute(transition_el);
    if (old_attr < old_max_attr + 1)
      parent_mesh.SetAttribute(transition_el, global_new_attrs[old_attr-1]);
  }

  mfem::AttributeSets & attr_sets = parent_mesh.attribute_sets;
  mfem::AttributeSets & bdr_attr_sets = parent_mesh.bdr_attribute_sets;
  /// Create attribute set labelling the newly created transition region
  /// on one side of the cut
  attr_sets.CreateAttributeSet(_transition_subdomain);
  /// Create attribute set labelling the entire closed geometry
  attr_sets.SetAttributeSet(_closed_subdomain, getSubdomainAttributes());
  /// Add the new domain attributes to new attribute sets
  for (int i = 0; i < global_new_attrs.Size(); ++i)
  {
    int attr = global_new_attrs[i];
    if (attr>old_max_attr)
    {
      attr_sets.AddToAttributeSet(_transition_subdomain, attr);
      attr_sets.AddToAttributeSet(_closed_subdomain, attr);    
    }
  }  

  /// Create boundary attribute set labelling the exterior of the newly created 
  /// transition region, excluding the cut  
  bdr_attr_sets.CreateAttributeSet(_transition_subdomain_boundary);
  bdr_attr_sets.AddToAttributeSet(_transition_subdomain_boundary,
                                  parent_mesh.bdr_attributes.Max() + 1);

  parent_mesh.SetAttributes();
}

bool
MFEMCutTransitionSubMesh::isInDomain(const int & element,
                                     const mfem::Array<int> & subdomains,
                                     const mfem::ParMesh & mesh)
{
  bool is_in_domain = false;
  /// element<0 for ghost elements
  if (element < 0)
    return true;

  for (const auto & subdomain : subdomains)
  {
    if (mesh.GetAttribute(element) == subdomain)
      is_in_domain = true;
  }
  return is_in_domain;
}

/// 3D Plane constructor and methods

Plane3D::Plane3D() : normal(3), d(0) {}

void
Plane3D::make3DPlane(const mfem::ParMesh & mesh, const int & face)
{
  MFEM_ASSERT(pm->Dimension() == 3, "Plane3D only works in 3-dimensional meshes!");

  mfem::Array<int> face_verts;
  std::vector<mfem::Vector> v;
  mesh.GetFaceVertices(face, face_verts);

  /// First we get the coordinates of 3 vertices on the face
  for (auto vtx : face_verts)
  {
    mfem::Vector vtx_coords(3);
    for (int j = 0; j < 3; ++j)
      vtx_coords[j] = mesh.GetVertex(vtx)[j];
    v.push_back(vtx_coords);
  }

  /// Now we find the unit vector normal to the face
  v[0] -= v[1];
  v[1] -= v[2];
  v[0].cross3D(v[1], normal);
  normal /= normal.Norml2();

  /// Finally, we find d:
  d = normal * v[2];
}

int
Plane3D::side(const mfem::Vector & v)
{
  double tol = 1e-8;
  double val = normal * v - d;
  // double val = v[1];
  if (val > tol)
    return 1;
  else if (val < -tol)
    return -1;
  else
    return 0;
}

#endif
