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

// Pushes an element into a vector if the vector does not yet contain that same
// element
template <typename T> void pushIfUnique(std::vector<T> &vec, const T el) {

  bool verify = true;

  for (auto e : vec) {
    if (e == el)
      verify = false;
  }

  if (verify == true)
    vec.push_back(el);
}

// Deletes and clears a vector of pointers
template <typename T> void deleteAndClear(std::vector<T *> v) {

  for (auto p : v)
    delete p;
  v.clear();
}

bool MFEMCutTransitionSubMesh::isInDomain(const int el, const mfem::Array<int> &dom,
  const mfem::ParMesh *mesh) {

// This is for ghost elements
if (el < 0)
return false;

bool verify = false;

for (auto sd : dom) {
if (mesh->GetAttribute(el) == sd)
verify = true;
}

return verify;
}

bool MFEMCutTransitionSubMesh::isInDomain(const int el, const int &sd,
  const mfem::ParMesh *mesh) {

// This is for ghost elements
if (el < 0)
return false;

return mesh->GetAttribute(el) == sd;
}

mfem::Vector MFEMCutTransitionSubMesh::elementCentre(int el, mfem::ParMesh *pm) {

mfem::Array<int> elem_vtx;
mfem::Vector com(3);
com = 0.0;

pm->GetElementVertices(el, elem_vtx);

for (auto vtx : elem_vtx) {
for (int j = 0; j < 3; ++j)
com[j] += pm->GetVertex(vtx)[j] / (double)elem_vtx.Size();
}

return com;
}

// 3D Plane constructor and methods

Plane3D::Plane3D() : d(0) {

  u = new mfem::Vector(3);
  *u = 0.0;
}

Plane3D::~Plane3D() { delete u; }

void Plane3D::make3DPlane(const mfem::ParMesh *pm, const int face) {

  MFEM_ASSERT(pm->Dimension() == 3,
              "Plane3D only works in 3-dimensional meshes!");

  mfem::Array<int> face_vtx;
  std::vector<mfem::Vector> v;
  pm->GetFaceVertices(face, face_vtx);

  // First we get the coordinates of 3 vertices on the face
  for (auto vtx : face_vtx) {
    mfem::Vector vtx_coords(3);
    for (int j = 0; j < 3; ++j)
      vtx_coords[j] = pm->GetVertex(vtx)[j];
    v.push_back(vtx_coords);
  }

  // Now we find the unit vector normal to the face
  v[0] -= v[1];
  v[1] -= v[2];
  v[0].cross3D(v[1], *u);
  *u /= u->Norml2();

  // Finally, we find d:
  d = *u * v[2];
}

int Plane3D::side(const mfem::Vector v) {
  double val = *u * v - d;

  if (val > 0)
    return 1;
  else if (val < 0)
    return -1;
  else
    return 0;
}

registerMooseObject("MooseApp", MFEMCutTransitionSubMesh);

InputParameters
MFEMCutTransitionSubMesh::validParams()
{
  InputParameters params = MFEMSubMesh::validParams();
  params += MFEMBlockRestrictable::validParams();
  params.addClassDescription("Class to construct an MFEMSubMesh formed from the subspace of the "
                             "parent mesh restricted to the set of user-specified subdomains.");
  params.addParam<BoundaryName>(
      "cut_boundary",
      "-1",
      "The boundary associated with the mesh cut.");
  params.addParam<BoundaryName>(
      "transition_subdomain_boundary",
      "-1",
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
    _transition_subdomain(getParam<SubdomainName>("transition_subdomain")),   
    _closed_subdomain(getParam<SubdomainName>("closed_subdomain")),    
    _subdomain_label(getMFEMProblem().mesh().getMFEMParMesh().attributes.Max()+1)
{
}

void
MFEMCutTransitionSubMesh::buildSubMesh()
{ 
  modifyMesh(getMFEMProblem().mesh().getMFEMParMesh());
  _submesh = std::make_shared<mfem::ParSubMesh>(mfem::ParSubMesh::CreateFromDomain(
    getMFEMProblem().mesh().getMFEMParMesh(), mfem::Array<int>({_subdomain_label})));
}

void 
MFEMCutTransitionSubMesh::modifyMesh(mfem::ParMesh & parent_mesh)
{
  std::vector<int> old_dom_attrs;
  std::vector<int> bdr_els;
  std::pair<int, int> elec_attrs_;
  elec_attrs_.first = _cut_bdr_attribute;
  elec_attrs_.second = parent_mesh.bdr_attributes.Max() + 1;

  // First we save the current domain attributes so they may be restored later
  for (int e = 0; e < parent_mesh.GetNE(); ++e)
    old_dom_attrs.push_back(parent_mesh.GetAttribute(e));

  // Now we need to find the electrode boundary
  for (int i = 0; i < parent_mesh.GetNBE(); ++i) {
    if (parent_mesh.GetBdrAttribute(i) == elec_attrs_.first) {
      bdr_els.push_back(i);
    }
  }

  Plane3D plane;

  if (bdr_els.size() > 0) {
    plane.make3DPlane(&parent_mesh, parent_mesh.GetBdrElementFaceIndex(bdr_els[0]));
  }

  std::vector<int> elec_vtx;
  // Create a vector containing all of the vertices on the electrode
  for (auto b_fc : bdr_els) {

    mfem::Array<int> face_vtx;
    parent_mesh.GetFaceVertices(parent_mesh.GetBdrElementFaceIndex(b_fc), face_vtx);

    for (auto v : face_vtx)
      pushIfUnique(elec_vtx, v);
  }

  // Now we need to find all elements in the mesh that touch, on at least one
  // vertex, the electrode face if they do touch the vertex, are on one side of
  // the electrode, and belong to the coil domain, we add them to our wedge

  std::vector<int> wedge_els;

  for (int e = 0; e < parent_mesh.GetNE(); ++e) {

    if (!isInDomain(e, getSubdomainAttributes(), &parent_mesh) ||
        plane.side(elementCentre(e, &parent_mesh)) == 1)
      continue;

    if (
    plane.side(elementCentre(e, &parent_mesh)) == 1)
  continue;

    mfem::Array<int> elem_vtx;
    parent_mesh.GetElementVertices(e, elem_vtx);

    for (auto v1 : elem_vtx) {
      for (auto v2 : elec_vtx) {
        if (v1 == v2) {
          pushIfUnique(wedge_els, e);
        }
      }
    }
  }

  // Now we set the second electrode boundary attribute. Start with a list of
  // all the faces of the wedge elements and eliminate mesh and coil boundaries,
  // the first electrode, and faces between wedge elements
  std::vector<int> wedge_faces;
  mfem::Array<int> el_faces;
  mfem::Array<int> ori;

  for (auto e : wedge_els) {
    parent_mesh.GetElementFaces(e, el_faces, ori);
    for (auto f : el_faces)
      pushIfUnique(wedge_faces, f);
  }

  for (auto wf : wedge_faces) {
    int e1, e2;
    parent_mesh.GetFaceElements(wf, &e1, &e2);

    // If the face is a coil boundary
    if (!(isInDomain(e1, getSubdomainAttributes(), &parent_mesh) &&
          isInDomain(e2, getSubdomainAttributes(), &parent_mesh))) {
      continue;
    }

    // If the face is not true interior
    if (!(parent_mesh.FaceIsInterior(wf) ||
          (parent_mesh.GetFaceInformation(wf).tag ==
               mfem::Mesh::FaceInfoTag::SharedConforming ||
           parent_mesh.GetFaceInformation(wf).tag ==
               mfem::Mesh::FaceInfoTag::SharedSlaveNonconforming))) {
      continue;
    }

    // If the face is shared between two elements internal to the wedge
    bool test1 = false;
    bool test2 = false;
    for (auto e : wedge_els) {
      if (e == e1)
        test1 = true;
      if (e == e2)
        test2 = true;
    }

    if (test1 && test2)
      continue;

    // If the face is part of the first electrode
    test1 = false;
    for (auto b_fc : bdr_els) {
      if (wf == parent_mesh.GetBdrElementFaceIndex(b_fc)) {
        test1 = true;
        break;
      }
    }
    if (test1)
      continue;

    // At last, if the face is none of these things, it must be our second
    // electrode
    auto *new_elem = parent_mesh.GetFace(wf)->Duplicate(&parent_mesh);
    new_elem->SetAttribute(elec_attrs_.second);
    parent_mesh.AddBdrElement(new_elem);
  }

  // Only after this do we set the domain attributes
  for (auto e : wedge_els)
    parent_mesh.SetAttribute(e, _subdomain_label);
  mfem::AttributeSets &attr_sets = parent_mesh.attribute_sets;
  /// Create attribute set labelling the newly created transition region 
  /// on one side of the cut
  attr_sets.CreateAttributeSet(_transition_subdomain);
  attr_sets.AddToAttributeSet(_transition_subdomain, _subdomain_label);
  
  /// Create attribute set labelling the entire closed geometry
  attr_sets.SetAttributeSet(_closed_subdomain, getSubdomainAttributes());
  attr_sets.AddToAttributeSet(_closed_subdomain, _subdomain_label);

  parent_mesh.FinalizeTopology();
  parent_mesh.Finalize();
  parent_mesh.SetAttributes();
}

#endif
