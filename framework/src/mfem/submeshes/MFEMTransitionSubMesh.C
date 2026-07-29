//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMTransitionSubMesh.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMTransitionSubMesh);
registerMooseObjectRenamed("MooseApp",
                           MFEMCutTransitionSubMesh,
                           "07/23/2027 00:00",
                           MFEMTransitionSubMesh);

InputParameters
MFEMTransitionSubMesh::validParams()
{
  InputParameters params = MFEMSubMesh::validParams();
  params += MFEMBlockRestrictable::validParams();
  params.addClassDescription(
      "Class to construct an MFEMSubMesh formed from the set of elements that have at least one "
      "vertex on the specified boundary, that lie on one side of it (the single interior side "
      "for an exterior boundary), and that are restricted to the set of user-specified "
      "subdomains.");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "cut_boundary",
      "The boundary or boundaries from which the transition region is constructed. Accepts a "
      "space-separated list of numeric boundary attributes and/or named boundary attribute sets.");
  params.deprecateParam("cut_boundary", "boundary", "07/23/2027");
  params.addRequiredParam<BoundaryName>(
      "transition_subdomain_boundary",
      "Name to assign boundary of transition subdomain not shared with the boundary surface.");
  params.addRequiredParam<SubdomainName>(
      "transition_subdomain",
      "The name of the subdomain to be created on the mesh comprised of the set of elements "
      "adjacent to the boundary on one side.");
  params.addRequiredParam<SubdomainName>(
      "closed_subdomain",
      "The name of the subdomain attribute to be created comprised of the set of all elements "
      "of the closed geometry, including the new transition region.");
  params.addRangeCheckedParam<unsigned int>(
      "num_layers_positive",
      1,
      "num_layers_positive >= 0",
      "Number of element-thick layers to grow on the positive side of the boundary (the side the "
      "surface normal points towards). Zero grows no layer on that side. For an exterior boundary "
      "this is the number of layers grown inwards.");
  params.addRangeCheckedParam<unsigned int>(
      "num_layers_negative",
      0,
      "num_layers_negative >= 0",
      "Number of element-thick layers to grow on the negative side of the boundary. Zero grows no "
      "layer on that side. Must be zero for an exterior boundary, which has only one side.");
  return params;
}

MFEMTransitionSubMesh::MFEMTransitionSubMesh(const InputParameters & parameters)
  : MFEMSubMesh(parameters),
    MFEMBlockRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh()),
    _boundary_submesh(std::make_shared<mfem::ParSubMesh>(
        mfem::ParSubMesh::CreateFromBoundary(getMesh(), boundaryAttributes()))),
    _transition_subdomain_boundary(getParam<BoundaryName>("transition_subdomain_boundary")),
    _transition_subdomain(getParam<SubdomainName>("transition_subdomain")),
    _closed_subdomain(getParam<SubdomainName>("closed_subdomain")),
    _num_layers_positive(getParam<unsigned int>("num_layers_positive")),
    _num_layers_negative(getParam<unsigned int>("num_layers_negative"))
{
}

mfem::Array<int>
MFEMTransitionSubMesh::boundaryAttributes()
{
  mfem::Array<int> attributes;
  for (const auto & name : getParam<std::vector<BoundaryName>>("boundary"))
  {
    try
    {
      // Numeric boundary attribute.
      attributes.Append(std::stoi(name));
    }
    catch (const std::invalid_argument &)
    {
      // Named boundary attribute set.
      attributes.Append(getMesh().bdr_attribute_sets.GetAttributeSet(name));
    }
  }
  return attributes;
}

void
MFEMTransitionSubMesh::buildSubMesh()
{
  labelMesh(const_cast<mfem::ParMesh &>(getMesh()));
  _submesh = std::make_shared<mfem::ParSubMesh>(mfem::ParSubMesh::CreateFromDomain(
      getMesh(), getMesh().attribute_sets.GetAttributeSet(_transition_subdomain)));
  _submesh->attribute_sets.attr_sets = getMesh().attribute_sets.attr_sets;
  _submesh->bdr_attribute_sets.attr_sets = getMesh().bdr_attribute_sets.attr_sets;
  // Create boundary attribute set labelling the exterior of the newly created
  // transition region, excluding the cut
  _submesh->bdr_attribute_sets.SetAttributeSet(
      _transition_subdomain_boundary, mfem::Array<int>({getMesh().bdr_attributes.Max() + 1}));
}

void
MFEMTransitionSubMesh::labelMesh(mfem::ParMesh & parent_mesh)
{
  const int mpi_comm_size = getMFEMProblem().getProblemData().num_procs;

  // Determine whether the supplied boundary is on the mesh exterior. A boundary face is
  // exterior only if its face topology is Boundary; interior cut faces and parallel
  // processor-shared faces both report as interior. The classification is reduced across
  // ranks so it is globally consistent.
  const mfem::Array<int> & parent_bdr_id_map = _boundary_submesh->GetParentElementIDMap();
  int local_interior_face_found = 0;
  for (const auto i : make_range(parent_bdr_id_map.Size()))
  {
    const int face = parent_mesh.GetBdrElementFaceIndex(parent_bdr_id_map[i]);
    if (!parent_mesh.GetFaceInformation(face).IsBoundary())
    {
      local_interior_face_found = 1;
      break;
    }
  }
  MPI_Allreduce(MPI_IN_PLACE,
                &local_interior_face_found,
                1,
                MPI_INT,
                MPI_MAX,
                getMFEMProblem().getComm());
  _exterior_boundary = (local_interior_face_found == 0);

  if (_exterior_boundary && _num_layers_negative > 0)
    mooseError("MFEMTransitionSubMesh: num_layers_negative must be zero for an exterior boundary, "
               "which has elements on one side only.");

  mfem::Array<HYPRE_BigInt> global_vertex_ids;
  parent_mesh.GetGlobalVertexIndices(global_vertex_ids);
  std::unique_ptr<mfem::Table> vert_to_elem(parent_mesh.GetVertexToElementTable());

  // Averaged surface normals at the boundary vertices, which identify both the boundary vertices
  // themselves and the side each neighbouring element lies on. An exterior boundary has elements
  // on one side only, so there only the set of boundary vertices is needed.
  std::map<HYPRE_BigInt, mfem::Vector> vertex_normals;
  if (_exterior_boundary)
  {
    const mfem::Array<int> & vertex_id_map = _boundary_submesh->GetParentVertexIDMap();
    std::vector<HYPRE_BigInt> ids_local;
    for (const auto i : make_range(_boundary_submesh->GetNV()))
      ids_local.push_back(global_vertex_ids[vertex_id_map[i]]);

    // Share the ids so that boundary vertices owned by other ranks are also seen here.
    int n_local = ids_local.size();
    std::vector<int> sizes(mpi_comm_size);
    MPI_Allgather(&n_local, 1, MPI_INT, sizes.data(), 1, MPI_INT, getMFEMProblem().getComm());
    std::vector<int> offset(mpi_comm_size);
    std::exclusive_scan(sizes.begin(), sizes.end(), offset.begin(), 0);
    const int total = std::accumulate(sizes.begin(), sizes.end(), 0);
    std::vector<HYPRE_BigInt> ids_all(total);
    MPI_Allgatherv(ids_local.data(),
                   n_local,
                   HYPRE_MPI_BIG_INT,
                   ids_all.data(),
                   sizes.data(),
                   offset.data(),
                   HYPRE_MPI_BIG_INT,
                   getMFEMProblem().getComm());
    for (const auto id : ids_all)
      vertex_normals[id];
  }
  else
    vertex_normals = computeVertexNormals(parent_mesh, global_vertex_ids);

  // Seed each side with the subdomain-restricted elements touching a boundary vertex.
  mfem::Array<int> positive_seed, negative_seed;
  for (const auto v : make_range(parent_mesh.GetNV()))
  {
    const auto it = vertex_normals.find(global_vertex_ids[v]);
    if (it == vertex_normals.end())
      continue;
    const int ne = vert_to_elem->RowSize(v);
    const int * els = vert_to_elem->GetRow(v);
    for (const auto i : make_range(ne))
    {
      const int el = els[i];
      if (el < 0 || !isInDomain(el, getSubdomainAttributes(), parent_mesh))
        continue;
      if (_exterior_boundary || isPositiveSide(el, v, it->second, parent_mesh))
        positive_seed.Append(el);
      else
        negative_seed.Append(el);
    }
  }

  mfem::Array<int> transition_els;
  std::set<int> transition_set;
  growLayers(parent_mesh,
             global_vertex_ids,
             *vert_to_elem,
             vertex_normals,
             positive_seed,
             _num_layers_positive,
             true,
             transition_set,
             transition_els);
  growLayers(parent_mesh,
             global_vertex_ids,
             *vert_to_elem,
             vertex_normals,
             negative_seed,
             _num_layers_negative,
             false,
             transition_set,
             transition_els);

  transition_els.Sort();
  transition_els.Unique();

  setAttributes(parent_mesh, transition_els);
}

mfem::Vector
MFEMTransitionSubMesh::findFaceNormal(const mfem::ParMesh & mesh, const int & face)
{
  if (mesh.SpaceDimension() != 3)
    mooseError("MFEMTransitionSubMesh only works in 3-dimensional meshes!");
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

std::map<HYPRE_BigInt, mfem::Vector>
MFEMTransitionSubMesh::computeVertexNormals(mfem::ParMesh & parent_mesh,
                                            const mfem::Array<HYPRE_BigInt> & global_vertex_ids)
{
  const int mpi_comm_size = getMFEMProblem().getProblemData().num_procs;

  // Accumulate the normal of every local boundary face into each of its vertices. Orientation
  // consistency between neighbouring faces comes from the mesh's own orientation of the surface,
  // which is well defined for an orientable boundary.
  std::map<HYPRE_BigInt, mfem::Vector> local_normals;
  const mfem::Array<int> & parent_bdr_id_map = _boundary_submesh->GetParentElementIDMap();
  for (const auto i : make_range(parent_bdr_id_map.Size()))
  {
    const int bdr_el = parent_bdr_id_map[i];
    const mfem::Vector n = findFaceNormal(parent_mesh, parent_mesh.GetBdrElementFaceIndex(bdr_el));
    mfem::Array<int> verts;
    parent_mesh.GetBdrElementVertices(bdr_el, verts);
    for (const auto v : verts)
    {
      auto & accumulated = local_normals[global_vertex_ids[v]];
      if (accumulated.Size() == 0)
      {
        accumulated.SetSize(3);
        accumulated = 0.0;
      }
      for (const auto d : make_range(3))
        accumulated[d] += n[d];
    }
  }

  // Flatten for communication: one global id and three normal components per entry.
  std::vector<HYPRE_BigInt> ids_local;
  std::vector<double> normals_local;
  for (const auto & [id, n] : local_normals)
  {
    ids_local.push_back(id);
    for (const auto d : make_range(3))
      normals_local.push_back(n[d]);
  }

  int n_local = ids_local.size();
  std::vector<int> id_sizes(mpi_comm_size);
  MPI_Allgather(&n_local, 1, MPI_INT, id_sizes.data(), 1, MPI_INT, getMFEMProblem().getComm());
  std::vector<int> id_offset(mpi_comm_size), normal_sizes(mpi_comm_size),
      normal_offset(mpi_comm_size);
  std::exclusive_scan(id_sizes.begin(), id_sizes.end(), id_offset.begin(), 0);
  for (const auto r : make_range(mpi_comm_size))
    normal_sizes[r] = 3 * id_sizes[r];
  std::exclusive_scan(normal_sizes.begin(), normal_sizes.end(), normal_offset.begin(), 0);
  const int n_total = std::accumulate(id_sizes.begin(), id_sizes.end(), 0);

  std::vector<HYPRE_BigInt> ids_all(n_total);
  std::vector<double> normals_all(3 * n_total);
  MPI_Allgatherv(ids_local.data(),
                 n_local,
                 HYPRE_MPI_BIG_INT,
                 ids_all.data(),
                 id_sizes.data(),
                 id_offset.data(),
                 HYPRE_MPI_BIG_INT,
                 getMFEMProblem().getComm());
  MPI_Allgatherv(normals_local.data(),
                 3 * n_local,
                 MPI_DOUBLE,
                 normals_all.data(),
                 normal_sizes.data(),
                 normal_offset.data(),
                 MPI_DOUBLE,
                 getMFEMProblem().getComm());

  // Sum contributions per global vertex, then normalise.
  std::map<HYPRE_BigInt, mfem::Vector> vertex_normals;
  for (const auto i : make_range(n_total))
  {
    auto & accumulated = vertex_normals[ids_all[i]];
    if (accumulated.Size() == 0)
    {
      accumulated.SetSize(3);
      accumulated = 0.0;
    }
    for (const auto d : make_range(3))
      accumulated[d] += normals_all[3 * i + d];
  }
  for (auto & [_, n] : vertex_normals)
  {
    const double norm = n.Norml2();
    if (norm > 0.0)
      n /= norm;
  }
  return vertex_normals;
}

void
MFEMTransitionSubMesh::growLayers(mfem::ParMesh & parent_mesh,
                                  const mfem::Array<HYPRE_BigInt> & global_vertex_ids,
                                  const mfem::Table & vert_to_elem,
                                  const std::map<HYPRE_BigInt, mfem::Vector> & vertex_normals,
                                  const mfem::Array<int> & seed,
                                  unsigned int num_layers,
                                  bool positive_side,
                                  std::set<int> & transition_set,
                                  mfem::Array<int> & transition_els)
{
  if (num_layers == 0)
    return;

  const int mpi_comm_size = getMFEMProblem().getProblemData().num_procs;

  mfem::Array<int> ring;
  for (const auto el : seed)
    if (transition_set.insert(el).second)
    {
      transition_els.Append(el);
      ring.Append(el);
    }

  for (unsigned int layer = 1; layer < num_layers; ++layer)
  {
    // Global vertex ids of the most recently added ring, shared across all ranks so that growth
    // crosses processor boundaries.
    std::vector<HYPRE_BigInt> front_local;
    for (const auto el : ring)
    {
      mfem::Array<int> el_verts;
      parent_mesh.GetElementVertices(el, el_verts);
      for (const auto v : el_verts)
        front_local.push_back(global_vertex_ids[v]);
    }

    int n_front = front_local.size();
    std::vector<int> front_sizes(mpi_comm_size);
    MPI_Allgather(
        &n_front, 1, MPI_INT, front_sizes.data(), 1, MPI_INT, getMFEMProblem().getComm());
    std::vector<int> front_offset(mpi_comm_size);
    std::exclusive_scan(front_sizes.begin(), front_sizes.end(), front_offset.begin(), 0);
    const int front_total = std::accumulate(front_sizes.begin(), front_sizes.end(), 0);
    std::vector<HYPRE_BigInt> front_all(front_total);
    MPI_Allgatherv(front_local.data(),
                   n_front,
                   HYPRE_MPI_BIG_INT,
                   front_all.data(),
                   front_sizes.data(),
                   front_offset.data(),
                   HYPRE_MPI_BIG_INT,
                   getMFEMProblem().getComm());
    std::sort(front_all.begin(), front_all.end());
    front_all.erase(std::unique(front_all.begin(), front_all.end()), front_all.end());

    mfem::Array<int> new_ring;
    for (const auto v : make_range(parent_mesh.GetNV()))
    {
      if (!std::binary_search(front_all.begin(), front_all.end(), global_vertex_ids[v]))
        continue;
      // A seed vertex lying on the boundary must not let growth cross to the other side.
      const auto it = vertex_normals.find(global_vertex_ids[v]);
      const bool on_boundary = (it != vertex_normals.end());
      const int ne = vert_to_elem.RowSize(v);
      const int * els = vert_to_elem.GetRow(v);
      for (const auto i : make_range(ne))
      {
        const int el = els[i];
        if (el < 0 || !isInDomain(el, getSubdomainAttributes(), parent_mesh))
          continue;
        if (on_boundary && !_exterior_boundary &&
            isPositiveSide(el, v, it->second, parent_mesh) != positive_side)
          continue;
        if (transition_set.insert(el).second)
        {
          transition_els.Append(el);
          new_ring.Append(el);
        }
      }
    }
    ring = new_ring;
  }
}

bool
MFEMTransitionSubMesh::isPositiveSide(const int & el,
                                      const int & boundary_vertex,
                                      const mfem::Vector & normal,
                                      mfem::ParMesh & parent_mesh)
{
  const int sdim = parent_mesh.SpaceDimension();
  mfem::Vector el_center(3);
  parent_mesh.GetElementCenter(el, el_center);
  mfem::Vector vertex_coords(parent_mesh.GetVertex(boundary_vertex), sdim);
  double dot = 0.0;
  for (const auto j : make_range(sdim))
    dot += normal[j] * (el_center[j] - vertex_coords[j]);
  return dot > 0;
}

void
MFEMTransitionSubMesh::setAttributes(mfem::ParMesh & parent_mesh,
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

  parent_mesh.SetAttributes();
}

bool
MFEMTransitionSubMesh::isInDomain(const int & element,
                                     const mfem::Array<int> & subdomains,
                                     const mfem::ParMesh & mesh)
{
  // element<0 for ghost elements
  if (element < 0)
    return true;

  // An empty subdomain list means the object applies to all subdomains.
  if (subdomains.Size() == 0)
    return true;

  for (const auto & subdomain : subdomains)
    if (mesh.GetAttribute(element) == subdomain)
      return true;
  return false;
}

#endif
