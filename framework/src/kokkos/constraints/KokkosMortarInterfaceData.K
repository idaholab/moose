//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMortarInterfaceData.h"

#include "AutomaticMortarGeneration.h"
#include "FEProblemBase.h"
#include "MortarUtils.h"

#include "libmesh/fe_map.h"
#include "libmesh/quadrature.h"
#include "libmesh/reference_elem.h"

namespace Moose::Kokkos
{

MortarInterfaceData::MortarInterfaceData(FEProblemBase & problem,
                                         const AutomaticMortarGeneration & amg)
  : MortarExecutorInterface(problem), _problem(problem), _amg(amg)
{
}

void
MortarInterfaceData::mortarSetup(const AutomaticMortarGeneration & amg)
{
  if (&amg == &_amg)
    _needs_packing = true;
}

const MortarInterfaceGeometry &
MortarInterfaceData::update(const bool sensitivities)
{
  // A displaced interface rebuilds its mortar segment mesh as the displacements move it
  if (_needs_packing || _amg.buildCount() != _packed_build_count)
  {
    packSegments();
    if (_need_normals)
      packNodalNormals();
    _needs_packing = false;
    _packed_build_count = _amg.buildCount();
    ++_revision;
  }

  if (sensitivities && _need_normals && _sensitivity_revision != _revision)
  {
    packSensitivities();
    _sensitivity_revision = _revision;
  }

  return _geometry;
}

void
MortarInterfaceData::packSegments()
{
  const auto & mesh = _problem.mesh();
  const auto & kokkos_mesh = *mesh.getKokkosMesh();
  auto & assembly = _problem.assembly(0, 0);
  const auto & qrule = assembly.qRuleMortar();
  const auto & JxW_msm = assembly.jxWMortar();
  const auto & coord_msm = assembly.mortarCoordTransformation();
  const auto secondary_sub_elem_index =
      _amg.dim() == 3 ? _amg.mortarSegmentMesh().get_elem_integer_index("secondary_sub_elem") : 0;
  const auto primary_sub_elem_index =
      _amg.dim() == 3 ? _amg.mortarSegmentMesh().get_elem_integer_index("primary_sub_elem") : 0;

  std::vector<dof_id_type> secondary_qp_begin = {0};
  std::vector<ContiguousElementID> elems[3];
  std::vector<libMesh::ElemType> types[3];
  std::vector<Point> xis[3];
  std::vector<Real> JxW;
  std::vector<Point> gap_vector, normal;

  _local_secondary_elems.clear();
  _amg_secondary_elems.clear();

  // The reference coordinates on an interior parent of a reference point on its lower-dimensional
  // side element, whose nodes are the nodes of that side in order
  const auto parent_point = [](const Elem & lower, const Point & xi)
  {
    const auto parent = lower.interior_parent();
    const auto side = parent->which_side_am_i(&lower);
    const auto & reference_parent = libMesh::ReferenceElem::get(parent->type());
    const auto reference_side = reference_parent.build_side_ptr(side);
    return libMesh::FEMap::map(lower.dim(), reference_side.get(), xi);
  };

  for (const auto & [secondary_id, msm_elems] : _amg.secondariesToMortarSegments())
  {
    // The map also lists secondary elements of other processes, which a distributed mesh need not
    // hold
    const auto secondary = mesh.getMesh().query_elem_ptr(secondary_id);
    if (!secondary || secondary->processor_id() != _problem.processor_id() || msm_elems.empty())
      continue;

    _local_secondary_elems.push_back(secondary);
    _amg_secondary_elems.push_back(
        libmesh_map_find(_amg.mortarSegmentMeshElemToInfo(), *msm_elems.begin()).secondary_elem);

    for (const auto msm_elem : msm_elems)
    {
      _problem.reinitMortarElem(msm_elem, 0);
      const auto & msinfo = libmesh_map_find(_amg.mortarSegmentMeshElemToInfo(), msm_elem);

      std::vector<Point> xi1, xi2;
      if (msm_elem->dim() == 1)
        for (const auto qp : make_range(qrule->n_points()))
        {
          const Real eta = qrule->qp(qp)(0);
          xi1.push_back(0.5 * (1 - eta) * msinfo.xi1_a + 0.5 * (1 + eta) * msinfo.xi1_b);
          xi2.push_back(0.5 * (1 - eta) * msinfo.xi2_a + 0.5 * (1 + eta) * msinfo.xi2_b);
        }
      else if (_amg.mortar3DQpMapping() == Mortar3DQuadraturePointMapping::REFERENCE_INTERPOLATION)
        Moose::Mortar::mapQPoints3dFromReference(
            *msm_elem, _amg.mortarSegmentReferencePoints(*msm_elem), *qrule, xi1, xi2);
      else
      {
        Moose::Mortar::projectQPoints3d(
            msm_elem, msinfo.secondary_elem, secondary_sub_elem_index, *qrule, xi1);
        Moose::Mortar::projectQPoints3d(
            msm_elem, msinfo.primary_elem, primary_sub_elem_index, *qrule, xi2);
      }

      // The segment elements of the current configuration belong to the displaced mesh, whose
      // elements share their IDs with the reference mesh that the Kokkos mesh mirrors
      const Elem * const side_elems[3] = {msinfo.secondary_elem->interior_parent(),
                                          msinfo.primary_elem->interior_parent(),
                                          msinfo.secondary_elem};
      const Elem * reference_elems[3];
      for (const auto side : make_range(3u))
        reference_elems[side] = mesh.elemPtr(side_elems[side]->id());

      const auto normals = _amg.getNormals(*msinfo.secondary_elem, xi1);

      for (const auto qp : make_range(qrule->n_points()))
      {
        const Point side_xi[3] = {parent_point(*msinfo.secondary_elem, xi1[qp]),
                                  parent_point(*msinfo.primary_elem, xi2[qp]),
                                  xi1[qp]};

        for (const auto side : make_range(3u))
        {
          elems[side].push_back(kokkos_mesh.getContiguousElementID(reference_elems[side]));
          types[side].push_back(side_elems[side]->type());
          xis[side].push_back(side_xi[side]);
        }

        JxW.push_back(JxW_msm[qp] * coord_msm[qp]);

        const auto x_secondary =
            libMesh::FEMap::map(side_elems[0]->dim(), side_elems[0], side_xi[0]);
        const auto x_primary = libMesh::FEMap::map(side_elems[1]->dim(), side_elems[1], side_xi[1]);
        gap_vector.push_back(x_primary - x_secondary);
        normal.push_back(normals[qp]);
      }
    }

    secondary_qp_begin.push_back(JxW.size());
  }

  auto & g = _geometry;
  const auto n = JxW.size();

  g.secondary_qp_begin = secondary_qp_begin;
  g.JxW = JxW;

  g.gap_vector.create(n);
  g.normal.create(n);
  for (const auto qp : make_range(n))
  {
    g.gap_vector[qp] = gap_vector[qp];
    g.normal[qp] = normal[qp];
  }
  g.gap_vector.copyToDevice();
  g.normal.copyToDevice();

  for (const auto side : make_range(3u))
  {
    g.elem[side] = elems[side];
    g.type[side] = types[side];

    g.xi[side].create(n);
    for (const auto qp : make_range(n))
      g.xi[side][qp] = xis[side][qp];
    g.xi[side].copyToDevice();
  }
}

void
MortarInterfaceData::packNodalNormals()
{
  const auto & kokkos_mesh = *_problem.mesh().getKokkosMesh();

  // The normals of the nodes of the local secondary elements, in the order of the elements
  std::unordered_map<const Node *, dof_id_type> normal_index;
  _normal_nodes.clear();
  std::vector<dof_id_type> elem_node_normal(_local_secondary_elems.size() *
                                                MortarInterfaceGeometry::max_face_nodes,
                                            libMesh::DofObject::invalid_id);
  std::vector<ContiguousNodeID> normal_node;
  std::vector<dof_id_type> normal_elem;
  std::vector<unsigned int> normal_elem_node;
  std::vector<Point> nodal_normal;

  for (const auto e : index_range(_local_secondary_elems))
  {
    const auto & elem = *_local_secondary_elems[e];
    mooseAssert(elem.n_nodes() <= MortarInterfaceGeometry::max_face_nodes,
                "Too many nodes on a secondary face");

    const auto normals = _amg.getNodalNormals(*_amg_secondary_elems[e]);

    for (const auto k : make_range(elem.n_nodes()))
    {
      const auto node = elem.node_ptr(k);
      const auto [it, inserted] = normal_index.emplace(node, _normal_nodes.size());
      if (inserted)
      {
        _normal_nodes.push_back(node);
        normal_node.push_back(kokkos_mesh.getContiguousNodeID(node));
        normal_elem.push_back(e);
        normal_elem_node.push_back(k);
        nodal_normal.push_back(normals[k]);
      }
      elem_node_normal[MortarInterfaceGeometry::max_face_nodes * e + k] = it->second;
    }
  }

  auto & g = _geometry;
  g.elem_node_normal = elem_node_normal;
  g.normal_node = normal_node;
  g.normal_elem = normal_elem;
  g.normal_elem_node = normal_elem_node;

  g.nodal_normal.create(nodal_normal.size());
  for (const auto i : index_range(nodal_normal))
    g.nodal_normal[i] = nodal_normal[i];
  g.nodal_normal.copyToDevice();
}

void
MortarInterfaceData::packSensitivities()
{
  const auto & mesh = _problem.mesh();
  const auto & kokkos_mesh = *mesh.getKokkosMesh();
  const auto normals = Moose::Mortar::computeNodalNormalSensitivities(_amg);
  const auto & node_to_secondary_elems = _amg.nodesToSecondaryElem();

  std::vector<dof_id_type> sensitivity_begin = {0};
  std::vector<ContiguousElementID> sensitivity_elem;
  std::vector<unsigned int> sensitivity_elem_node;
  std::vector<Real33> sensitivity;

  for (const auto node : _normal_nodes)
  {
    for (const auto & [dependency, tensor] : libmesh_map_find(normals, node->id()).sensitivities)
    {
      // The normal depends only on the nodes of the secondary faces, each of which lies on an
      // interior parent of the secondary surface
      const auto & lower_elems = libmesh_map_find(node_to_secondary_elems, dependency);
      mooseAssert(lower_elems.size(), "A secondary node lies on no secondary element");
      const auto parent = mesh.elemPtr(lower_elems.front()->interior_parent()->id());
      sensitivity_elem.push_back(kokkos_mesh.getContiguousElementID(parent));
      sensitivity_elem_node.push_back(parent->get_node_index(mesh.nodePtr(dependency)));
      sensitivity.push_back(tensor);
    }
    sensitivity_begin.push_back(sensitivity.size());
  }

  auto & g = _geometry;
  g.sensitivity_begin = sensitivity_begin;
  g.sensitivity_elem = sensitivity_elem;
  g.sensitivity_elem_node = sensitivity_elem_node;

  g.sensitivity.create(sensitivity.size());
  for (const auto i : index_range(sensitivity))
    g.sensitivity[i] = sensitivity[i];
  g.sensitivity.copyToDevice();
}

} // namespace Moose::Kokkos
