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

GPUAssembly::GPUAssembly(FEProblemBase & problem)
  : GPUMeshHolder(*problem.mesh().getGPUMesh()), _problem(problem), _mesh(problem.mesh())
{
}

void
GPUAssembly::init()
{
  // Cache mesh information

  _n_subdomains = _mesh.meshSubdomains().size();
  _n_elem_types = mesh().getGPUElementTypeMap().size();
  _dimension = _mesh.dimension();

  _coord_type.create(_n_subdomains);

  for (auto subdomain : _mesh.meshSubdomains())
    _coord_type[mesh().getGPUSubdomainID(subdomain)] = _mesh.getCoordSystem(subdomain);

  _coord_type.copy();

  if (_mesh.usingGeneralAxisymmetricCoordAxes())
  {
    _rz_axis.create(_n_subdomains);

    for (auto subdomain : _mesh.meshSubdomains())
      _rz_axis[mesh().getGPUSubdomainID(subdomain)] =
          _mesh.getGeneralAxisymmetricCoordAxis(subdomain);

    _rz_axis.copy();
  }
  else
    _rz_radial_coord = _mesh.getAxisymmetricRadialCoord();

  // Generate the list of unique FE types

  std::set<FEType> fe_types;

  auto getFETypes = [&](System & system)
  {
    for (unsigned int var = 0; var < system.n_vars(); var++)
      fe_types.insert(system.variable_type(var));
  };

  for (unsigned int nl = 0; nl < _problem.numNonlinearSystems(); ++nl)
    getFETypes(_problem.getNonlinearSystemBase(nl).system());

  getFETypes(_problem.getAuxiliarySystem().system());

  for (auto & fet : fe_types)
    _fe_type_map[fet] = _fe_type_map.size();
}

void
GPUAssembly::initQuadrature()
{
  _q_points.create(_n_subdomains, _n_elem_types);
  _q_points_face.create(_n_subdomains, _n_elem_types);
  _weights.create(_n_subdomains, _n_elem_types);
  _weights_face.create(_n_subdomains, _n_elem_types);

  _n_qps.create(_n_subdomains);
  _n_qps_face.create(_n_subdomains);
  _qp_offset.create(_n_subdomains);
  _qp_offset_face.create(_n_subdomains);
  _max_qps_per_elem.create(_n_subdomains + 1);
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
      mooseError("GPUAssembly error: ", object->name(), " is not boundary-restricted.");
  }

  // Cache quadrature data

  for (auto subdomain : _mesh.meshSubdomains())
  {
    auto sid = mesh().getGPUSubdomainID(subdomain);

    std::map<ElemType, unsigned int> n_qps;
    std::map<ElemType, std::vector<unsigned int>> n_qps_face;

    auto & assembly = _problem.assembly(0, 0);
    auto & qrule = assembly.writeableQRule(_dimension, subdomain);
    auto & qrule_face = assembly.writeableQRuleFace(_dimension, subdomain);

    for (auto & [elem_type, elem_type_id] : mesh().getGPUElementTypeMap())
    {
      auto elem = &libMesh::ReferenceElem::get(elem_type);

      _q_points_face(sid, elem_type_id).create(elem->n_sides());
      _weights_face(sid, elem_type_id).create(elem->n_sides());

      // Cache volume quadrature of each reference element

      qrule->init(elem->type(), /* p-level */ 0);
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
        qrule_face->init(elem->side_ptr(side)->type(), /* p-level */ 0);
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

    auto & map = mesh().getGPUSubdomainLocalElementMap(subdomain);

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
        auto sid = mesh().getGPUSubdomainID(elem->subdomain_id());
        auto eid = mesh().getGPUSubdomainLocalElementID(elem);
        auto side = _mesh.sideWithBoundaryID(elem, boundary);

        _qp_offset_face[sid](side, eid) = _n_qps_face[sid](side, eid);
      }
    }

  for (auto subdomain : _mesh.meshSubdomains())
  {
    auto sid = mesh().getGPUSubdomainID(subdomain);

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
GPUAssembly::initShape()
{
  // Cache reference shape data

  _phi.create(_n_subdomains, _n_elem_types, _fe_type_map.size());
  _phi_face.create(_n_subdomains, _n_elem_types, _fe_type_map.size());
  _grad_phi.create(_n_subdomains, _n_elem_types, _fe_type_map.size());
  _grad_phi_face.create(_n_subdomains, _n_elem_types, _fe_type_map.size());

  _map_phi.create(_n_subdomains, _n_elem_types);
  _map_phi_face.create(_n_subdomains, _n_elem_types);
  _map_psi_face.create(_n_subdomains, _n_elem_types);
  _map_grad_phi.create(_n_subdomains, _n_elem_types);
  _map_grad_phi_face.create(_n_subdomains, _n_elem_types);
  _map_grad_psi_face.create(_n_subdomains, _n_elem_types);

  _n_dofs.create(_n_elem_types, _fe_type_map.size());
  _n_dofs = 0;

  for (auto subdomain : _mesh.meshSubdomains())
  {
    auto sid = mesh().getGPUSubdomainID(subdomain);

    for (unsigned int t = 0; t < _n_elem_types; ++t)
    {
      _map_phi(sid, t).create(MAX_DOF, _max_qps_per_elem[sid]);
      _map_phi_face(sid, t).create(MAX_DOF, _max_qps_per_elem[sid], _mesh.getMaxSidesPerElem());
      _map_psi_face(sid, t).create(MAX_DOF, _max_qps_per_elem[sid], _mesh.getMaxSidesPerElem());
      _map_grad_phi(sid, t).create(MAX_DOF, _max_qps_per_elem[sid]);
      _map_grad_phi_face(sid, t).create(
          MAX_DOF, _max_qps_per_elem[sid], _mesh.getMaxSidesPerElem());
      _map_grad_psi_face(sid, t).create(
          MAX_DOF, _max_qps_per_elem[sid], _mesh.getMaxSidesPerElem());

      for (auto & [fe_type, fe_idx] : _fe_type_map)
      {
        _phi(sid, t, fe_idx).create(MAX_DOF, _max_qps_per_elem[sid]);
        _phi_face(sid, t, fe_idx)
            .create(MAX_DOF, _max_qps_per_elem[sid], _mesh.getMaxSidesPerElem());
        _grad_phi(sid, t, fe_idx).create(MAX_DOF, _max_qps_per_elem[sid]);
        _grad_phi_face(sid, t, fe_idx)
            .create(MAX_DOF, _max_qps_per_elem[sid], _mesh.getMaxSidesPerElem());

        _phi(sid, t, fe_idx) = 0;
        _phi_face(sid, t, fe_idx) = 0;
        _grad_phi(sid, t, fe_idx) = Real3(0);
        _grad_phi_face(sid, t, fe_idx) = Real3(0);
      }
    }
  }

  for (auto subdomain : _mesh.meshSubdomains())
  {
    auto sid = mesh().getGPUSubdomainID(subdomain);
    auto & map = mesh().getGPUSubdomainLocalElementMap(subdomain);

    auto & assembly = _problem.assembly(0, 0);
    auto & qrule = assembly.writeableQRule(_dimension, subdomain);
    auto & qrule_face = assembly.writeableQRuleFace(_dimension, subdomain);

    for (auto & [fe_type, fe_type_id] : _fe_type_map)
    {
      std::unique_ptr<FEBase> fe(FEBase::build(_dimension, fe_type));
      std::unique_ptr<FEBase> fe_face(FEBase::build(_dimension, fe_type));

      fe->attach_quadrature_rule(qrule.get());
      fe_face->attach_quadrature_rule(qrule_face.get());

      for (auto & [elem_type, elem_type_id] : mesh().getGPUElementTypeMap())
      {
        auto elem = &libMesh::ReferenceElem::get(elem_type);

        auto & phi = fe->get_phi();
        auto & grad_phi = fe->get_dphi();

        fe->reinit(elem);

        _n_dofs(elem_type_id, fe_type_id) = phi.size();

        for (unsigned int i = 0; i < phi.size(); ++i)
          for (unsigned int qp = 0; qp < qrule->n_points(); ++qp)
          {
            _phi(sid, elem_type_id, fe_type_id)(i, qp) = phi[i][qp];
            _grad_phi(sid, elem_type_id, fe_type_id)(i, qp) = grad_phi[i][qp];
          }

        for (unsigned int side = 0; side < elem->n_sides(); ++side)
        {
          auto & phi = fe_face->get_phi();
          auto & grad_phi = fe_face->get_dphi();

          fe_face->reinit(elem, side);

          for (unsigned int i = 0; i < phi.size(); ++i)
            for (unsigned int qp = 0; qp < qrule_face->n_points(); ++qp)
            {
              _phi_face(sid, elem_type_id, fe_type_id)(i, qp, side) = phi[i][qp];
              _grad_phi_face(sid, elem_type_id, fe_type_id)(i, qp, side) = grad_phi[i][qp];
            }
        }
      }
    }

    for (auto & [elem_type, elem_type_id] : mesh().getGPUElementTypeMap())
    {
      auto elem = &libMesh::ReferenceElem::get(elem_type);

      std::unique_ptr<FEBase> fe(
          FEBase::build(_dimension, FEType(elem->default_order(), LAGRANGE)));
      std::unique_ptr<FEBase> fe_face(
          FEBase::build(_dimension, FEType(elem->default_order(), LAGRANGE)));

      fe->attach_quadrature_rule(qrule.get());
      fe_face->attach_quadrature_rule(qrule_face.get());

      auto & phi = fe->get_phi();
      auto & grad_phi = fe->get_dphi();

      fe->reinit(elem);

      for (unsigned int i = 0; i < phi.size(); ++i)
        for (unsigned int qp = 0; qp < qrule->n_points(); ++qp)
        {
          _map_phi(sid, elem_type_id)(i, qp) = phi[i][qp];
          _map_grad_phi(sid, elem_type_id)(i, qp) = grad_phi[i][qp];
        }

      for (unsigned int side = 0; side < elem->n_sides(); ++side)
      {
        auto & phi = fe_face->get_phi();
        auto & grad_phi = fe_face->get_dphi();

        fe_face->reinit(elem, side);

        for (unsigned int i = 0; i < phi.size(); ++i)
          for (unsigned int qp = 0; qp < qrule_face->n_points(); ++qp)
          {
            _map_phi_face(sid, elem_type_id)(i, qp, side) = phi[i][qp];
            _map_grad_phi_face(sid, elem_type_id)(i, qp, side) = grad_phi[i][qp];
          }

        auto & psi = fe_face->get_fe_map().get_psi();
        auto & dpsidxi = fe_face->get_fe_map().get_dpsidxi();
        auto & dpsideta = fe_face->get_fe_map().get_dpsideta();

        for (unsigned int i = 0; i < psi.size(); ++i)
          for (unsigned int qp = 0; qp < qrule_face->n_points(); ++qp)
          {
            _map_psi_face(sid, elem_type_id)(i, qp, side) = psi[i][qp];
            if (elem->dim() > 1)
              _map_grad_psi_face(sid, elem_type_id)(i, qp, side)(0) = dpsidxi[i][qp];
            if (elem->dim() > 2)
              _map_grad_psi_face(sid, elem_type_id)(i, qp, side)(1) = dpsideta[i][qp];
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
