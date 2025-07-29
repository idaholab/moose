//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUAssembly.h"

#include "MooseMesh.h"
#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"
#include "Assembly.h"
#include "BoundaryRestrictable.h"

#include "libmesh/fe_interface.h"
#include "libmesh/reference_elem.h"

namespace Moose
{
namespace Kokkos
{

Assembly::Assembly(FEProblemBase & problem)
  : MeshHolder(*problem.mesh().getKokkosMesh()),
    _problem(problem),
    _mesh(problem.mesh()),
    _dimension(_mesh.dimension())
{
}

void
Assembly::init()
{
  // Cache mesh information

  auto num_subdomains = _mesh.meshSubdomains().size();

  _coord_type.create(num_subdomains);

  for (auto subdomain : _mesh.meshSubdomains())
    _coord_type[kokkosMesh().getSubdomainID(subdomain)] = _mesh.getCoordSystem(subdomain);

  _coord_type.copy();

  if (_mesh.usingGeneralAxisymmetricCoordAxes())
  {
    _rz_axis.create(num_subdomains);

    for (auto subdomain : _mesh.meshSubdomains())
      _rz_axis[kokkosMesh().getSubdomainID(subdomain)] =
          _mesh.getGeneralAxisymmetricCoordAxis(subdomain);

    _rz_axis.copy();
  }
  else
    _rz_radial_coord = _mesh.getAxisymmetricRadialCoord();

  // Initialize quadrature and shape data

  initQuadrature();
  initShape();
  cachePhysicalMap();
}

void
Assembly::initQuadrature()
{
  auto num_subdomains = _mesh.meshSubdomains().size();
  auto num_elem_types = kokkosMesh().getElementTypeMap().size();

  _q_points.create(num_subdomains, num_elem_types);
  _q_points_face.create(num_subdomains, num_elem_types);
  _weights.create(num_subdomains, num_elem_types);
  _weights_face.create(num_subdomains, num_elem_types);

  _n_qps.create(num_subdomains);
  _n_qps_face.create(num_subdomains);
  _qp_offset.create(num_subdomains);
  _qp_offset_face.create(num_subdomains);
  _max_qps_per_elem.create(num_subdomains + 1);
  _max_qps_per_elem = 0;

  // Find boundaries where material properties should be computed

  auto & boundary_objects =
      _problem.getMaterialPropertyStorageConsumers(Moose::BOUNDARY_MATERIAL_DATA, true);

  for (auto object : boundary_objects)
  {
    auto boundary_restriction = dynamic_cast<const BoundaryRestrictable *>(object);

    if (boundary_restriction)
      for (auto boundary : boundary_restriction->boundaryIDs())
        _material_boundaries.insert(boundary);
    else
      mooseError("Kokkos assembly error: ", object->name(), " is not boundary-restricted.");
  }

  // Cache quadrature data

  for (auto subdomain : _mesh.meshSubdomains())
  {
    auto sid = kokkosMesh().getSubdomainID(subdomain);

    std::map<ElemType, unsigned int> n_qps;
    std::map<ElemType, std::vector<unsigned int>> n_qps_face;

    auto & assembly = _problem.assembly(0, 0);
    auto qrule = assembly.writeableQRule(_dimension, subdomain, {});
    auto qrule_face = assembly.writeableQRuleFace(_dimension, subdomain, {});

    for (auto & [elem_type, elem_type_id] : kokkosMesh().getElementTypeMap())
    {
      auto elem = &libMesh::ReferenceElem::get(elem_type);

      _q_points_face(sid, elem_type_id).create(elem->n_sides());
      _weights_face(sid, elem_type_id).create(elem->n_sides());

      // Cache volume quadrature of each reference element

      qrule->init(*elem, /* p-level */ 0);
      n_qps[elem_type] = qrule->n_points();

      _q_points(sid, elem_type_id).create(qrule->n_points());
      _weights(sid, elem_type_id).create(qrule->n_points());

      for (unsigned int qp = 0; qp < qrule->n_points(); ++qp)
      {
        _q_points(sid, elem_type_id)[qp] = qrule->qp(qp);
        _weights(sid, elem_type_id)[qp] = qrule->w(qp);
      }

      // Cache face quadrature of each reference element

      for (unsigned int side = 0; side < elem->n_sides(); ++side)
      {
        qrule_face->init(*elem->side_ptr(side), /* p-level */ 0);
        n_qps_face[elem_type].push_back(qrule_face->n_points());

        _q_points_face(sid, elem_type_id)[side].create(qrule_face->n_points());
        _weights_face(sid, elem_type_id)[side].create(qrule_face->n_points());

        for (unsigned int qp = 0; qp < qrule_face->n_points(); ++qp)
        {
          _q_points_face(sid, elem_type_id)[side][qp] = qrule_face->qp(qp);
          _weights_face(sid, elem_type_id)[side][qp] = qrule_face->w(qp);
        }
      }
    }

    auto & map = kokkosMesh().getSubdomainLocalElementMap(subdomain);

    _n_qps[sid].create(map.size());
    _n_qps_face[sid].create(_mesh.getMaxSidesPerElem(), map.size());
    _n_qps_face[sid] = 0;

    _qp_offset[sid].create(map.size() + 1);
    _qp_offset_face[sid].create(_mesh.getMaxSidesPerElem(), map.size() + 1);
    _qp_offset_face[sid] = 0;

    for (auto & [elem, eid] : map)
    {
      _n_qps[sid][eid] = n_qps[elem->type()];
      _qp_offset[sid][eid] = n_qps[elem->type()];

      for (unsigned int side = 0; side < elem->n_sides(); ++side)
        _n_qps_face[sid](side, eid) = n_qps_face[elem->type()][side];

      _max_qps_per_elem[sid] = std::max(_max_qps_per_elem[sid], _n_qps[sid][eid]);
    }

    _max_qps_per_elem.last() = std::max(_max_qps_per_elem.last(), _max_qps_per_elem[sid]);

    std::exclusive_scan(_qp_offset[sid].begin(), _qp_offset[sid].end(), _qp_offset[sid].begin(), 0);
  }

  for (auto boundary : _material_boundaries)
    for (auto elem_id : _mesh.getBoundaryActiveSemiLocalElemIds(boundary))
    {
      auto elem = _mesh.elemPtr(elem_id);

      if (elem->processor_id() == _problem.processor_id())
      {
        auto sid = kokkosMesh().getSubdomainID(elem->subdomain_id());
        auto eid = kokkosMesh().getSubdomainLocalElementID(elem);
        auto side = _mesh.sideWithBoundaryID(elem, boundary);

        _qp_offset_face[sid](side, eid) = _n_qps_face[sid](side, eid);
      }
    }

  for (auto subdomain : _mesh.meshSubdomains())
  {
    auto sid = kokkosMesh().getSubdomainID(subdomain);

    std::exclusive_scan(
        _qp_offset_face[sid].begin(), _qp_offset_face[sid].end(), _qp_offset_face[sid].begin(), 0);
  }

  _q_points.copyNested();
  _q_points_face.copyNested();
  _weights.copyNested();
  _weights_face.copyNested();

  _n_qps.copyNested();
  _n_qps_face.copyNested();
  _qp_offset.copyNested();
  _qp_offset_face.copyNested();
  _max_qps_per_elem.copy();
}

void
Assembly::initShape()
{
  // Generate the list of unique FE types

  std::set<FEType> fe_types;

  auto getFETypes = [&](::System & system)
  {
    for (unsigned int var = 0; var < system.n_vars(); var++)
      fe_types.insert(system.variable_type(var));
  };

  for (unsigned int nl = 0; nl < _problem.numNonlinearSystems(); ++nl)
    getFETypes(_problem.getNonlinearSystemBase(nl).system());

  getFETypes(_problem.getAuxiliarySystem().system());

  _fe_type_map.clear();

  for (auto & fet : fe_types)
    _fe_type_map[fet] = _fe_type_map.size();

  // Cache reference shape data

  auto num_subdomains = _mesh.meshSubdomains().size();
  auto num_elem_types = kokkosMesh().getElementTypeMap().size();

  _phi.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _phi_face.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _grad_phi.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _grad_phi_face.create(num_subdomains, num_elem_types, _fe_type_map.size());

  _map_phi.create(num_subdomains, num_elem_types);
  _map_phi_face.create(num_subdomains, num_elem_types);
  _map_psi_face.create(num_subdomains, num_elem_types);
  _map_grad_phi.create(num_subdomains, num_elem_types);
  _map_grad_phi_face.create(num_subdomains, num_elem_types);
  _map_grad_psi_face.create(num_subdomains, num_elem_types);

  _n_dofs.create(num_elem_types, _fe_type_map.size());
  _n_dofs = 0;

  for (auto subdomain : _mesh.meshSubdomains())
  {
    auto sid = kokkosMesh().getSubdomainID(subdomain);

    auto & assembly = _problem.assembly(0, 0);
    auto qrule = assembly.writeableQRule(_dimension, subdomain, {});
    auto qrule_face = assembly.writeableQRuleFace(_dimension, subdomain, {});

    for (auto & [fe_type, fe_type_id] : _fe_type_map)
    {
      std::unique_ptr<FEBase> fe(FEBase::build(_dimension, fe_type));
      std::unique_ptr<FEBase> fe_face(FEBase::build(_dimension, fe_type));

      fe->attach_quadrature_rule(qrule);
      fe_face->attach_quadrature_rule(qrule_face);

      for (auto & [elem_type, elem_type_id] : kokkosMesh().getElementTypeMap())
      {
        auto elem = &libMesh::ReferenceElem::get(elem_type);

        auto & phi = fe->get_phi();
        auto & grad_phi = fe->get_dphi();

        fe->reinit(elem);

        _n_dofs(elem_type_id, fe_type_id) = phi.size();

        _phi(sid, elem_type_id, fe_type_id).create(phi.size(), qrule->n_points());
        _grad_phi(sid, elem_type_id, fe_type_id).create(phi.size(), qrule->n_points());

        for (unsigned int i = 0; i < phi.size(); ++i)
          for (unsigned int qp = 0; qp < qrule->n_points(); ++qp)
          {
            _phi(sid, elem_type_id, fe_type_id)(i, qp) = phi[i][qp];
            _grad_phi(sid, elem_type_id, fe_type_id)(i, qp) = grad_phi[i][qp];
          }

        _phi_face(sid, elem_type_id, fe_type_id).create(elem->n_sides());
        _grad_phi_face(sid, elem_type_id, fe_type_id).create(elem->n_sides());

        for (unsigned int side = 0; side < elem->n_sides(); ++side)
        {
          auto & phi = fe_face->get_phi();
          auto & grad_phi = fe_face->get_dphi();

          fe_face->reinit(elem, side);

          _phi_face(sid, elem_type_id, fe_type_id)(side).create(phi.size(), qrule_face->n_points());
          _grad_phi_face(sid, elem_type_id, fe_type_id)(side).create(phi.size(),
                                                                     qrule_face->n_points());

          for (unsigned int i = 0; i < phi.size(); ++i)
            for (unsigned int qp = 0; qp < qrule_face->n_points(); ++qp)
            {
              _phi_face(sid, elem_type_id, fe_type_id)(side)(i, qp) = phi[i][qp];
              _grad_phi_face(sid, elem_type_id, fe_type_id)(side)(i, qp) = grad_phi[i][qp];
            }
        }
      }
    }

    for (auto & [elem_type, elem_type_id] : kokkosMesh().getElementTypeMap())
    {
      auto elem = &libMesh::ReferenceElem::get(elem_type);

      std::unique_ptr<FEBase> fe(
          FEBase::build(_dimension, FEType(elem->default_order(), LAGRANGE)));
      std::unique_ptr<FEBase> fe_face(
          FEBase::build(_dimension, FEType(elem->default_order(), LAGRANGE)));

      fe->attach_quadrature_rule(qrule);
      fe_face->attach_quadrature_rule(qrule_face);

      auto & phi = fe->get_phi();
      auto & grad_phi = fe->get_dphi();

      fe->reinit(elem);

      _map_phi(sid, elem_type_id).create(phi.size(), qrule->n_points());
      _map_grad_phi(sid, elem_type_id).create(phi.size(), qrule->n_points());

      for (unsigned int i = 0; i < phi.size(); ++i)
        for (unsigned int qp = 0; qp < qrule->n_points(); ++qp)
        {
          _map_phi(sid, elem_type_id)(i, qp) = phi[i][qp];
          _map_grad_phi(sid, elem_type_id)(i, qp) = grad_phi[i][qp];
        }

      _map_phi_face(sid, elem_type_id).create(elem->n_sides());
      _map_grad_phi_face(sid, elem_type_id).create(elem->n_sides());
      _map_psi_face(sid, elem_type_id).create(elem->n_sides());
      _map_grad_psi_face(sid, elem_type_id).create(elem->n_sides());

      for (unsigned int side = 0; side < elem->n_sides(); ++side)
      {
        auto & phi = fe_face->get_phi();
        auto & grad_phi = fe_face->get_dphi();

        fe_face->reinit(elem, side);

        _map_phi_face(sid, elem_type_id)(side).create(phi.size(), qrule_face->n_points());
        _map_grad_phi_face(sid, elem_type_id)(side).create(phi.size(), qrule_face->n_points());

        for (unsigned int i = 0; i < phi.size(); ++i)
          for (unsigned int qp = 0; qp < qrule_face->n_points(); ++qp)
          {
            _map_phi_face(sid, elem_type_id)(side)(i, qp) = phi[i][qp];
            _map_grad_phi_face(sid, elem_type_id)(side)(i, qp) = grad_phi[i][qp];
          }

        auto & psi = fe_face->get_fe_map().get_psi();
        auto & dpsidxi = fe_face->get_fe_map().get_dpsidxi();
        auto & dpsideta = fe_face->get_fe_map().get_dpsideta();

        _map_psi_face(sid, elem_type_id)(side).create(psi.size(), qrule_face->n_points());
        _map_grad_psi_face(sid, elem_type_id)(side).create(psi.size(), qrule_face->n_points());

        for (unsigned int i = 0; i < psi.size(); ++i)
          for (unsigned int qp = 0; qp < qrule_face->n_points(); ++qp)
          {
            _map_psi_face(sid, elem_type_id)(side)(i, qp) = psi[i][qp];
            if (elem->dim() > 1)
              _map_grad_psi_face(sid, elem_type_id)(side)(i, qp)(0) = dpsidxi[i][qp];
            if (elem->dim() > 2)
              _map_grad_psi_face(sid, elem_type_id)(side)(i, qp)(1) = dpsideta[i][qp];
          }
      }
    }
  }

  _phi.copyNested();
  _phi_face.copyNested();
  _grad_phi.copyNested();
  _grad_phi_face.copyNested();

  _map_phi.copyNested();
  _map_phi_face.copyNested();
  _map_psi_face.copyNested();
  _map_grad_phi.copyNested();
  _map_grad_phi_face.copyNested();
  _map_grad_psi_face.copyNested();

  _n_dofs.copy();
}

void
Assembly::cachePhysicalMap()
{
  auto num_subdomains = _mesh.meshSubdomains().size();
  auto num_elems = kokkosMesh().getLocalElementMap().size();

  _jacobian.create(num_subdomains);
  _jxw.create(num_subdomains);
  _xyz.create(num_subdomains);

  for (auto subdomain : _mesh.meshSubdomains())
  {
    auto sid = kokkosMesh().getSubdomainID(subdomain);

    _jacobian[sid].createDevice(_qp_offset[sid].last());
    _jxw[sid].createDevice(_qp_offset[sid].last());
    _xyz[sid].createDevice(_qp_offset[sid].last());
  }

  _jacobian.copyNested();
  _jxw.copyNested();
  _xyz.copyNested();

  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<dof_id_type>> policy(0, num_elems);
  ::Kokkos::parallel_for(policy, *this);
  ::Kokkos::fence();
}

KOKKOS_FUNCTION void
Assembly::operator()(const dof_id_type tid) const
{
  auto info = kokkosMesh().getElementInfo(tid);
  auto offset = getQpOffset(info);

  auto jacobian = &_jacobian[info.subdomain][offset];
  auto jxw = &_jxw[info.subdomain][offset];
  auto xyz = &_xyz[info.subdomain][offset];

  for (unsigned int qp = 0; qp < getNumQps(info); ++qp)
    computePhysicalMap(info, qp, &jacobian[qp], &jxw[qp], &xyz[qp]);
}

} // namespace Kokkos
} // namespace Moose
