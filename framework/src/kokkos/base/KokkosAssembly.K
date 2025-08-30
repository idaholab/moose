//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosAssembly.h"

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

  for (auto subdomain : _mesh.meshSubdomains())
    _coord_type[subdomain] = _mesh.getCoordSystem(subdomain);

  _coord_type.copyToDevice();

  if (_mesh.usingGeneralAxisymmetricCoordAxes())
  {
    for (auto subdomain : _mesh.meshSubdomains())
      _rz_axis[subdomain] = _mesh.getGeneralAxisymmetricCoordAxis(subdomain);

    _rz_axis.copyToDevice();
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
  auto num_elem_types = kokkosMesh().getElementTypeMap().size();

  // Find boundaries where material properties should be computed

  auto & boundary_objects =
      _problem.getKokkosMaterialPropertyStorageConsumers(Moose::BOUNDARY_MATERIAL_DATA);

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

  _max_qps_per_elem = 0;

  for (auto subdomain : _mesh.meshSubdomains())
  {
    std::map<ElemType, unsigned int> n_qps;
    std::map<ElemType, std::vector<unsigned int>> n_qps_face;

    auto & assembly = _problem.assembly(0, 0);
    auto qrule = assembly.writeableQRule(_dimension, subdomain, {});
    auto qrule_face = assembly.writeableQRuleFace(_dimension, subdomain, {});

    _weights[subdomain].create(num_elem_types);
    _weights_face[subdomain].create(num_elem_types);

    for (auto & [elem_type, elem_type_id] : kokkosMesh().getElementTypeMap())
    {
      auto elem = &libMesh::ReferenceElem::get(elem_type);

      _weights_face[subdomain][elem_type_id].create(elem->n_sides());

      // Cache volume quadrature of each reference element

      qrule->init(*elem, /* p-level */ 0);
      n_qps[elem_type] = qrule->n_points();

      _weights[subdomain][elem_type_id].create(qrule->n_points());

      for (unsigned int qp = 0; qp < qrule->n_points(); ++qp)
        _weights[subdomain][elem_type_id][qp] = qrule->w(qp);

      // Cache face quadrature of each reference element

      for (unsigned int side = 0; side < elem->n_sides(); ++side)
      {
        qrule_face->init(*elem->side_ptr(side), /* p-level */ 0);
        n_qps_face[elem_type].push_back(qrule_face->n_points());

        _weights_face[subdomain][elem_type_id][side].create(qrule_face->n_points());

        for (unsigned int qp = 0; qp < qrule_face->n_points(); ++qp)
          _weights_face[subdomain][elem_type_id][side][qp] = qrule_face->w(qp);
      }
    }

    auto & map = kokkosMesh().getSubdomainLocalElementMap(subdomain);

    _n_qps[subdomain].create(map.size());
    _n_qps_face[subdomain].create(_mesh.getMaxSidesPerElem(), map.size());
    _n_qps_face[subdomain] = 0;

    _qp_offset[subdomain].create(map.size() + 1);
    _qp_offset_face[subdomain].create(_mesh.getMaxSidesPerElem(), map.size() + 1);
    _qp_offset_face[subdomain] = 0;

    for (auto & [elem, eid] : map)
    {
      _n_qps[subdomain][eid] = n_qps[elem->type()];
      _qp_offset[subdomain][eid] = n_qps[elem->type()];

      for (unsigned int side = 0; side < elem->n_sides(); ++side)
        _n_qps_face[subdomain](side, eid) = n_qps_face[elem->type()][side];

      _max_qps_per_elem = std::max(_max_qps_per_elem, _n_qps[subdomain][eid]);
    }

    std::exclusive_scan(_qp_offset[subdomain].begin(),
                        _qp_offset[subdomain].end(),
                        _qp_offset[subdomain].begin(),
                        0);
  }

  for (auto boundary : _material_boundaries)
    for (auto elem_id : _mesh.getBoundaryActiveSemiLocalElemIds(boundary))
    {
      auto elem = _mesh.elemPtr(elem_id);

      if (elem->processor_id() == _problem.processor_id())
      {
        auto sid = elem->subdomain_id();
        auto eid = kokkosMesh().getSubdomainLocalElementID(elem);
        auto side = _mesh.sideWithBoundaryID(elem, boundary);

        _qp_offset_face[sid](side, eid) = _n_qps_face[sid](side, eid);
      }
    }

  for (auto subdomain : _mesh.meshSubdomains())
    std::exclusive_scan(_qp_offset_face[subdomain].begin(),
                        _qp_offset_face[subdomain].end(),
                        _qp_offset_face[subdomain].begin(),
                        0);

  _weights.copyToDeviceNested();
  _weights_face.copyToDeviceNested();

  _n_qps.copyToDeviceNested();
  _n_qps_face.copyToDeviceNested();
  _qp_offset.copyToDeviceNested();
  _qp_offset_face.copyToDeviceNested();
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

  auto num_elem_types = kokkosMesh().getElementTypeMap().size();

  _n_dofs.create(num_elem_types, _fe_type_map.size());
  _n_dofs = 0;

  for (auto subdomain : _mesh.meshSubdomains())
  {
    auto & assembly = _problem.assembly(0, 0);
    auto qrule = assembly.writeableQRule(_dimension, subdomain, {});
    auto qrule_face = assembly.writeableQRuleFace(_dimension, subdomain, {});

    _phi[subdomain].create(num_elem_types, _fe_type_map.size());
    _phi_face[subdomain].create(num_elem_types, _fe_type_map.size());
    _grad_phi[subdomain].create(num_elem_types, _fe_type_map.size());
    _grad_phi_face[subdomain].create(num_elem_types, _fe_type_map.size());

    _map_phi[subdomain].create(num_elem_types);
    _map_phi_face[subdomain].create(num_elem_types);
    _map_psi_face[subdomain].create(num_elem_types);
    _map_grad_phi[subdomain].create(num_elem_types);
    _map_grad_phi_face[subdomain].create(num_elem_types);
    _map_grad_psi_face[subdomain].create(num_elem_types);

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

        _phi[subdomain](elem_type_id, fe_type_id).create(phi.size(), qrule->n_points());
        _grad_phi[subdomain](elem_type_id, fe_type_id).create(phi.size(), qrule->n_points());

        for (unsigned int i = 0; i < phi.size(); ++i)
          for (unsigned int qp = 0; qp < qrule->n_points(); ++qp)
          {
            _phi[subdomain](elem_type_id, fe_type_id)(i, qp) = phi[i][qp];
            _grad_phi[subdomain](elem_type_id, fe_type_id)(i, qp) = grad_phi[i][qp];
          }

        _phi_face[subdomain](elem_type_id, fe_type_id).create(elem->n_sides());
        _grad_phi_face[subdomain](elem_type_id, fe_type_id).create(elem->n_sides());

        for (unsigned int side = 0; side < elem->n_sides(); ++side)
        {
          auto & phi = fe_face->get_phi();
          auto & grad_phi = fe_face->get_dphi();

          fe_face->reinit(elem, side);

          _phi_face[subdomain](elem_type_id, fe_type_id)(side).create(phi.size(),
                                                                      qrule_face->n_points());
          _grad_phi_face[subdomain](elem_type_id, fe_type_id)(side).create(phi.size(),
                                                                           qrule_face->n_points());

          for (unsigned int i = 0; i < phi.size(); ++i)
            for (unsigned int qp = 0; qp < qrule_face->n_points(); ++qp)
            {
              _phi_face[subdomain](elem_type_id, fe_type_id)(side)(i, qp) = phi[i][qp];
              _grad_phi_face[subdomain](elem_type_id, fe_type_id)(side)(i, qp) = grad_phi[i][qp];
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

      _map_phi[subdomain][elem_type_id].create(phi.size(), qrule->n_points());
      _map_grad_phi[subdomain][elem_type_id].create(phi.size(), qrule->n_points());

      for (unsigned int i = 0; i < phi.size(); ++i)
        for (unsigned int qp = 0; qp < qrule->n_points(); ++qp)
        {
          _map_phi[subdomain][elem_type_id](i, qp) = phi[i][qp];
          _map_grad_phi[subdomain][elem_type_id](i, qp) = grad_phi[i][qp];
        }

      _map_phi_face[subdomain][elem_type_id].create(elem->n_sides());
      _map_grad_phi_face[subdomain][elem_type_id].create(elem->n_sides());
      _map_psi_face[subdomain][elem_type_id].create(elem->n_sides());
      _map_grad_psi_face[subdomain][elem_type_id].create(elem->n_sides());

      for (unsigned int side = 0; side < elem->n_sides(); ++side)
      {
        auto & phi = fe_face->get_phi();
        auto & grad_phi = fe_face->get_dphi();

        fe_face->reinit(elem, side);

        _map_phi_face[subdomain][elem_type_id][side].create(phi.size(), qrule_face->n_points());
        _map_grad_phi_face[subdomain][elem_type_id][side].create(phi.size(),
                                                                 qrule_face->n_points());

        for (unsigned int i = 0; i < phi.size(); ++i)
          for (unsigned int qp = 0; qp < qrule_face->n_points(); ++qp)
          {
            _map_phi_face[subdomain][elem_type_id][side](i, qp) = phi[i][qp];
            _map_grad_phi_face[subdomain][elem_type_id][side](i, qp) = grad_phi[i][qp];
          }

        auto & psi = fe_face->get_fe_map().get_psi();
        auto & dpsidxi = fe_face->get_fe_map().get_dpsidxi();
        auto & dpsideta = fe_face->get_fe_map().get_dpsideta();

        _map_psi_face[subdomain][elem_type_id][side].create(psi.size(), qrule_face->n_points());
        _map_grad_psi_face[subdomain][elem_type_id][side].create(psi.size(),
                                                                 qrule_face->n_points());

        for (unsigned int i = 0; i < psi.size(); ++i)
          for (unsigned int qp = 0; qp < qrule_face->n_points(); ++qp)
          {
            _map_psi_face[subdomain][elem_type_id][side](i, qp) = psi[i][qp];
            if (elem->dim() > 1)
              _map_grad_psi_face[subdomain][elem_type_id][side](i, qp)(0) = dpsidxi[i][qp];
            if (elem->dim() > 2)
              _map_grad_psi_face[subdomain][elem_type_id][side](i, qp)(1) = dpsideta[i][qp];
          }
      }
    }
  }

  _phi.copyToDeviceNested();
  _phi_face.copyToDeviceNested();
  _grad_phi.copyToDeviceNested();
  _grad_phi_face.copyToDeviceNested();

  _map_phi.copyToDeviceNested();
  _map_phi_face.copyToDeviceNested();
  _map_psi_face.copyToDeviceNested();
  _map_grad_phi.copyToDeviceNested();
  _map_grad_phi_face.copyToDeviceNested();
  _map_grad_psi_face.copyToDeviceNested();

  _n_dofs.copyToDevice();
}

void
Assembly::cachePhysicalMap()
{
  auto num_elems = kokkosMesh().getLocalElementMap().size();

  for (auto subdomain : _mesh.meshSubdomains())
  {
    _jacobian[subdomain].createDevice(_qp_offset[subdomain].last());
    _jxw[subdomain].createDevice(_qp_offset[subdomain].last());
    _xyz[subdomain].createDevice(_qp_offset[subdomain].last());
  }

  _jacobian.copyToDevice();
  _jxw.copyToDevice();
  _xyz.copyToDevice();

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
