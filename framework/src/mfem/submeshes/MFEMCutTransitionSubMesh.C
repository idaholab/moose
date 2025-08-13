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
    getMesh(), getMFEMProblem().mesh().getMFEMParMesh().attribute_sets.GetAttributeSet(_transition_subdomain) ));
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

  /// Iterate over all vertices on cut, find elements with those vertices,
  /// and declare them transition elements if they are on the +ve side of the cut
  mfem::Array<int> transition_els;
  mfem::Table *vert_to_elem  = parent_mesh.GetVertexToElementTable();
  const mfem::Array<int> & cut_to_parent_vertex_id_map = _cut_submesh->GetParentVertexIDMap();
  for (int i = 0; i < _cut_submesh->GetNV(); ++i)
  {
    int cut_vert = cut_to_parent_vertex_id_map[i];
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

  // Detect shared vertices and add corresponding elements 
  for (int g = 1, sv = 0; g < parent_mesh.GetNGroups(); g++)
  {
    for (int gv = 0; gv < parent_mesh.GroupNVertices(g); gv++, sv++)
    {
    int plvtx = parent_mesh.GroupVertex(g, gv);
    const int sdim = parent_mesh.SpaceDimension();
    mfem::Vector coord(sdim);
    coord = parent_mesh.GetVertex(plvtx);
    if (plane.side(coord) == 0) // check if shared vertex is on the cut plane
    {
      int cut_vert = plvtx;
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
  mfem::AttributeSets &attr_sets = parent_mesh.attribute_sets;
  mfem::AttributeSets &bdr_attr_sets = parent_mesh.bdr_attribute_sets;
  const int old_max_attr = parent_mesh.attributes.Max();

  mfem::Array<int> new_attrs(old_max_attr);
  for (int i = 0; i < new_attrs.Size(); ++i)
  {
    new_attrs[i] = i+1;
  }  

  for (int i = 0; i < transition_els.Size(); ++i)
  {
    const auto & transition_el = transition_els[i];
    const int old_attr = parent_mesh.GetAttribute(transition_el);
    new_attrs[old_attr-1] = old_max_attr + old_attr%old_max_attr;
  }

  mfem::Array<int> global_new_attrs(old_max_attr);
  global_new_attrs = -1;
  MPI_Allreduce(new_attrs, global_new_attrs, old_max_attr,
              MPI_INT, MPI_MAX, getMFEMProblem().mesh().getMFEMParMesh().GetComm());

  for (int i = 0; i < transition_els.Size(); ++i)
  {
    const auto & transition_el = transition_els[i];
    const int old_attr = parent_mesh.GetAttribute(transition_el);
    if (old_attr < old_max_attr + 1)
      parent_mesh.SetAttribute(transition_el, global_new_attrs[old_attr-1]);
  }

  /// Set the domain attributes for the transition region
  attr_sets.CreateAttributeSet(_transition_subdomain);
  attr_sets.SetAttributeSet(_closed_subdomain, getSubdomainAttributes());
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

  /// Create attribute set labelling the newly created transition region 
  /// on one side of the cut
  
  /// Create attribute set labelling the entire closed geometry

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
    return false;

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
  double tol = 1e-12;
  double val = normal * v - d;
  if (val > tol)
    return 1;
  else if (val < -tol)
    return -1;
  else
    return 0;
}

#endif
