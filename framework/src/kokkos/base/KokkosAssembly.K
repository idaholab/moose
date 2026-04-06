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

// ── Native FE path headers ────────────────────────────────────────────────────
// Included only when the native flag is active so that the default build has no
// new compile-time dependencies.
#ifdef MOOSE_KOKKOS_NATIVE_FE
#include "KokkosFELagrange1D.h"
#include "KokkosFELagrange2D.h"
#include "KokkosFELagrange3D.h"
#include "KokkosFEMonomial.h"
#include "KokkosFEMapEvaluator.h"
// KokkosFEReinit.h provides nativeShape(FEShapeKey) and nativeGradShape(FEShapeKey)
// used by the key-based evalShapeAtQp overload below.
#include "KokkosFEReinit.h"
#endif

// MOOSE_KOKKOS_ONDEMAND_FE (Phase 6) requires MOOSE_KOKKOS_NATIVE_FE.
#ifdef MOOSE_KOKKOS_ONDEMAND_FE
#ifndef MOOSE_KOKKOS_NATIVE_FE
#error "MOOSE_KOKKOS_ONDEMAND_FE requires MOOSE_KOKKOS_NATIVE_FE to also be defined"
#endif
#endif

#ifdef MOOSE_KOKKOS_NATIVE_QUADRATURE
#include "KokkosQuadrature.h"
#endif

namespace Moose::Kokkos
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

  const auto num_subdomains = kokkosMesh().getNumSubdomains();

  _coord_type.create(num_subdomains);

  for (auto subdomain : _mesh.meshSubdomains())
    _coord_type[kokkosMesh().getContiguousSubdomainID(subdomain)] = _mesh.getCoordSystem(subdomain);

  _coord_type.copyToDevice();

  if (_mesh.usingGeneralAxisymmetricCoordAxes())
  {
    _rz_axis.create(num_subdomains);

    for (auto subdomain : _mesh.meshSubdomains())
      _rz_axis[kokkosMesh().getContiguousSubdomainID(subdomain)] =
          _mesh.getGeneralAxisymmetricCoordAxis(subdomain);

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
  const auto num_subdomains = kokkosMesh().getNumSubdomains();
  const auto num_elem_types = kokkosMesh().getNumLocalElementTypes();

  _q_points.create(num_subdomains, num_elem_types);
  _q_points_face.create(num_subdomains, num_elem_types);
  _weights.create(num_subdomains, num_elem_types);
  _weights_face.create(num_subdomains, num_elem_types);

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

  std::map<SubdomainID, std::map<ElemType, unsigned int>> n_qps;
  std::map<SubdomainID, std::map<ElemType, std::vector<unsigned int>>> n_qps_face;

  _max_qps_per_elem = 0;

  for (auto subdomain : _mesh.meshSubdomains())
  {
    auto sid = kokkosMesh().getContiguousSubdomainID(subdomain);

    auto & assembly = _problem.assembly(0, 0);
    auto qrule = assembly.writeableQRule(_dimension, subdomain, {});
    auto qrule_face = assembly.writeableQRuleFace(_dimension, subdomain, {});

    for (auto & [elem_type, elem_type_id] : kokkosMesh().getElementTypeMap())
    {
      auto elem = &libMesh::ReferenceElem::get(elem_type);

      _q_points_face(sid, elem_type_id).create(elem->n_sides());
      _weights_face(sid, elem_type_id).create(elem->n_sides());

#ifdef MOOSE_KOKKOS_NATIVE_QUADRATURE
      {
        // Fill volume quadrature natively
        const auto topo = toKokkosTopology(elem_type);
        const auto vol_order = static_cast<unsigned int>(qrule->get_order());
        fillQuadrature(topo, vol_order, _q_points(sid, elem_type_id), _weights(sid, elem_type_id));
        n_qps[subdomain][elem_type] = _q_points(sid, elem_type_id).size();

        // Fill face quadrature natively
        const auto side_topo = getSideTopology(topo);
        const auto face_order = static_cast<unsigned int>(qrule_face->get_order());
        for (unsigned int side = 0; side < elem->n_sides(); ++side)
        {
          fillQuadrature(side_topo,
                         face_order,
                         _q_points_face(sid, elem_type_id)[side],
                         _weights_face(sid, elem_type_id)[side]);
          n_qps_face[subdomain][elem_type].push_back(
              _q_points_face(sid, elem_type_id)[side].size());
        }
      }
#else
      // Cache volume quadrature of each reference element

      qrule->init(*elem, /* p-level */ 0);
      n_qps[subdomain][elem_type] = qrule->n_points();

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
        n_qps_face[subdomain][elem_type].push_back(qrule_face->n_points());

        _q_points_face(sid, elem_type_id)[side].create(qrule_face->n_points());
        _weights_face(sid, elem_type_id)[side].create(qrule_face->n_points());

        for (unsigned int qp = 0; qp < qrule_face->n_points(); ++qp)
        {
          _q_points_face(sid, elem_type_id)[side][qp] = qrule_face->qp(qp);
          _weights_face(sid, elem_type_id)[side][qp] = qrule_face->w(qp);
        }
      }
#endif

      _max_qps_per_elem = std::max(_max_qps_per_elem, n_qps[subdomain][elem_type]);
    }
  }

  const auto num_elems = _mesh.nActiveLocalElem();

  _n_subdomain_qps.create(num_subdomains);
  _n_subdomain_qps_face.create(num_subdomains);
  _n_subdomain_qps = 0;
  _n_subdomain_qps_face = 0;

  _n_qps.create(num_elems);
  _n_qps_face.create(_mesh.getMaxSidesPerElem(), num_elems);
  _n_qps_face = 0;

  _qp_offset.create(num_elems);
  _qp_offset_face.create(_mesh.getMaxSidesPerElem(), num_elems);
  _qp_offset_face = libMesh::DofObject::invalid_id;

  _elem_face_property_idx.create(_mesh.getMaxSidesPerElem(), num_elems);
  _elem_face_property_idx = libMesh::DofObject::invalid_id;

  _n_elem_face_properties.create(num_subdomains);
  _n_elem_face_properties = 0;

  for (auto elem : *_mesh.getActiveLocalElementRange())
  {
    auto eid = kokkosMesh().getContiguousElementID(elem);
    auto sid = kokkosMesh().getContiguousSubdomainID(elem->subdomain_id());

    _n_qps[eid] = n_qps[elem->subdomain_id()][elem->type()];
    _qp_offset[eid] = _n_subdomain_qps[sid];
    _n_subdomain_qps[sid] += _n_qps[eid];

    for (unsigned int side = 0; side < elem->n_sides(); ++side)
      _n_qps_face(side, eid) = n_qps_face[elem->subdomain_id()][elem->type()][side];
  }

  for (auto boundary : _material_boundaries)
    for (auto elem_id : _mesh.getBoundaryActiveSemiLocalElemIds(boundary))
    {
      auto elem = _mesh.elemPtr(elem_id);

      if (elem->processor_id() == _problem.processor_id())
      {
        auto sid = kokkosMesh().getContiguousSubdomainID(elem->subdomain_id());
        auto eid = kokkosMesh().getContiguousElementID(elem);
        auto side = _mesh.sideWithBoundaryID(elem, boundary);

        _qp_offset_face(side, eid) = _n_subdomain_qps_face[sid];
        _n_subdomain_qps_face[sid] += _n_qps_face(side, eid);

        _elem_face_property_idx(side, eid) = _n_elem_face_properties[sid];
        ++_n_elem_face_properties[sid];
      }
    }

  _q_points.copyToDeviceNested();
  _q_points_face.copyToDeviceNested();
  _weights.copyToDeviceNested();
  _weights_face.copyToDeviceNested();

  _n_qps.copyToDevice();
  _n_qps_face.copyToDevice();
  _n_subdomain_qps.copyToDevice();
  _n_subdomain_qps_face.copyToDevice();
  _qp_offset.copyToDevice();
  _qp_offset_face.copyToDevice();

  _elem_face_property_idx.copyToDevice();
  _n_elem_face_properties.copyToDevice();
}

#ifdef MOOSE_KOKKOS_NATIVE_FE
static Real3 mapFaceQpToParent(const libMesh::Elem & parent,
                                unsigned int side,
                                FEElemTopology side_topo,
                                Real3 face_qpt);
#endif

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

  const auto num_subdomains = kokkosMesh().getNumSubdomains();
  const auto num_elem_types = kokkosMesh().getNumLocalElementTypes();

#ifdef MOOSE_KOKKOS_ONDEMAND_FE
  // On-demand FE: LAGRANGE and MONOMIAL variables use device-side shape evaluation
  // when the nDofs formula matches libMesh.  Unsupported families (HERMITE,
  // LAGRANGE_VEC, etc.) and order-mismatched Lagrange types fall back to the
  // precomputed phi-table path.

  // Build element-type → topology map for on-device evaluators.
  _elem_topologies.create(num_elem_types);
  for (auto & [elem_type, elem_type_id] : kokkosMesh().getElementTypeMap())
    _elem_topologies[elem_type_id] = toKokkosTopology(elem_type);
  _elem_topologies.copyToDevice();

  // Build per-(elem_type, fe_type) on-demand flag.  True when our nDofs(FEShapeKey)
  // formula agrees with libMesh's actual DOF count for that (topology, FE type) pair.
  // A lower-order Lagrange type on a higher-order element (e.g. FIRST on QUAD9)
  // will disagree, so it falls back to precomputed phi tables.
  // Filled inside the subdomain loop below; copied to device after the loop.
  _is_ondemand_fe_type.create(num_elem_types, _fe_type_map.size());

  // Build per-(elem_type, fe_type) shape key table.  Encodes (family, element
  // base class, order) for on-device dispatch in nativeShape(FEShapeKey).
  // Populated now (before the subdomain loop) since it depends only on topology
  // and FE type, not on quadrature or subdomain.
  _shape_keys.create(num_elem_types, _fe_type_map.size());
  for (auto & [elem_type, elem_type_id] : kokkosMesh().getElementTypeMap())
    for (auto & [fe_type, fe_type_id] : _fe_type_map)
    {
      const auto geom_topo     = toKokkosTopology(elem_type);
      const auto cls           = classFromTopology(geom_topo);
      const auto fam           = toKokkosFamily(fe_type.family);
      const unsigned int order = static_cast<unsigned int>(fe_type.order);
      _shape_keys(elem_type_id, fe_type_id) = FEShapeKey{fam, cls, order};
    }
  _shape_keys.copyToDevice();

  // Allocate the face-qp-in-parent-coords array; filled per-subdomain in the loop below.
  _q_points_face_parent.create(num_subdomains, num_elem_types);

  // Allocate phi tables for FE types that fall back to the precomputed table path
  // (unsupported families, order mismatches, etc.).
  _phi.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _phi_face.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _grad_phi.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _grad_phi_face.create(num_subdomains, num_elem_types, _fe_type_map.size());
#else
  _phi.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _phi_face.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _grad_phi.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _grad_phi_face.create(num_subdomains, num_elem_types, _fe_type_map.size());
#endif

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
    auto sid = kokkosMesh().getContiguousSubdomainID(subdomain);

    auto & assembly = _problem.assembly(0, 0);
    auto qrule = assembly.writeableQRule(_dimension, subdomain, {});
    auto qrule_face = assembly.writeableQRuleFace(_dimension, subdomain, {});

    // ── Physics DOF shape arrays ───────────────────────────────────────────────

    for (auto & [fe_type, fe_type_id] : _fe_type_map)
    {
      std::unique_ptr<FEBase> fe(FEBase::build(_dimension, fe_type));
      std::unique_ptr<FEBase> fe_face(FEBase::build(_dimension, fe_type));

      fe->attach_quadrature_rule(qrule);
      fe_face->attach_quadrature_rule(qrule_face);

      for (auto & [elem_type, elem_type_id] : kokkosMesh().getElementTypeMap())
      {
        auto elem = &libMesh::ReferenceElem::get(elem_type);

        // Three-way dispatch for physics shape arrays:
        //   Phase 6 (ONDEMAND): LAGRANGE and MONOMIAL — record DOF count only; evaluation on-device
        //                       Others — fall back to precomputed phi-table path
        //   Phase 5 (NATIVE):   LAGRANGE and MONOMIAL — evaluate on CPU into cached tables via initShapeNative
        //                       Others — evaluate on CPU via libMesh FEBase::reinit
        //   default:            evaluate on CPU via libMesh FEBase::reinit
        bool use_libmesh_path = true;

#ifdef MOOSE_KOKKOS_ONDEMAND_FE
        {
          // Activate on-demand for LAGRANGE and MONOMIAL when our nDofs(FEShapeKey)
          // formula agrees with the DOF count libMesh assigns for this (topology,
          // FE type) pair.  Order mismatches (e.g. FIRST on QUAD9) produce phi.size()
          // != native_ndofs and fall back to the phi-table path.  Other families
          // (HERMITE, LAGRANGE_VEC, etc.) also fall back unconditionally.
          const FEShapeKey key = _shape_keys(elem_type_id, fe_type_id);
          const bool family_supported = (key.family == FEFamily::LAGRANGE ||
                                         key.family == FEFamily::MONOMIAL);

          if (family_supported)
          {
            const unsigned int native_ndofs = nDofs(key);
            auto & phi = fe->get_phi();
            fe->get_dphi();
            fe->reinit(elem);
            if (phi.size() == native_ndofs)
            {
              _n_dofs(elem_type_id, fe_type_id) = native_ndofs;
              _is_ondemand_fe_type(elem_type_id, fe_type_id) = true;
              use_libmesh_path = false;
            }
            else
            {
              // nDofs formula disagrees with libMesh — fall back to table path
              _is_ondemand_fe_type(elem_type_id, fe_type_id) = false;
            }
          }
          else
          {
            _is_ondemand_fe_type(elem_type_id, fe_type_id) = false;
          }
        }
#elif defined(MOOSE_KOKKOS_NATIVE_FE)
        {
          const auto topo = toKokkosTopology(elem_type);
          const auto fam = toKokkosFamily(fe_type.family);
          const bool family_supported = (fam == FEFamily::LAGRANGE || fam == FEFamily::MONOMIAL);
          if (family_supported)
          {
            const FEShapeKey key{fam, classFromTopology(topo),
                                 static_cast<unsigned int>(fe_type.order)};
            initShapeNative(sid,
                            elem_type_id,
                            fe_type_id,
                            key,
                            topo,
                            qrule->n_points(),
                            qrule_face->n_points(),
                            *elem);
            use_libmesh_path = false;
          }
        }
#endif

        if (use_libmesh_path)
        {
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

            _phi_face(sid, elem_type_id, fe_type_id)(side).create(phi.size(),
                                                                   qrule_face->n_points());
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
    }

    // ── Geometric map arrays ───────────────────────────────────────────────────

    for (auto & [elem_type, elem_type_id] : kokkosMesh().getElementTypeMap())
    {
      auto elem = &libMesh::ReferenceElem::get(elem_type);

#ifdef MOOSE_KOKKOS_ONDEMAND_FE
      // Phase 6: precompute face qp parent-reference coordinates so on-device
      // shape evaluators can look up (xi, eta, zeta) without calling mapFaceQpToParent
      // at kernel time.  The mapping depends only on element topology, not physics.
      {
        const auto topo = toKokkosTopology(elem_type);
        const auto side_topo = getSideTopology(topo);
        const unsigned int n_sides = elem->n_sides();

        _q_points_face_parent(sid, elem_type_id).create(n_sides);

        for (unsigned int side = 0; side < n_sides; ++side)
        {
          const auto & face_qpts = _q_points_face(sid, elem_type_id)[side];
          const unsigned int n_face_qps = face_qpts.size();
          _q_points_face_parent(sid, elem_type_id)[side].create(n_face_qps);

          for (unsigned int qp = 0; qp < n_face_qps; ++qp)
            _q_points_face_parent(sid, elem_type_id)[side][qp] =
                mapFaceQpToParent(*elem, side, side_topo, face_qpts[qp]);
        }
      }
#endif

#ifdef MOOSE_KOKKOS_NATIVE_FE
      initMapNative(sid, elem_type_id, toKokkosTopology(elem_type), _dimension, *elem);
#else
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
#endif
    }
  }

#ifdef MOOSE_KOKKOS_ONDEMAND_FE
  // Copy parent-reference face qp table and per-(elem,fe) on-demand flags.
  _q_points_face_parent.copyToDeviceNested();
  _is_ondemand_fe_type.copyToDevice();
  // Copy phi tables for FE types that fall back to the precomputed table path.
  _phi.copyToDeviceNested();
  _phi_face.copyToDeviceNested();
  _grad_phi.copyToDeviceNested();
  _grad_phi_face.copyToDeviceNested();
#else
  _phi.copyToDeviceNested();
  _phi_face.copyToDeviceNested();
  _grad_phi.copyToDeviceNested();
  _grad_phi_face.copyToDeviceNested();
#endif

  _map_phi.copyToDeviceNested();
  _map_phi_face.copyToDeviceNested();
  _map_psi_face.copyToDeviceNested();
  _map_grad_phi.copyToDeviceNested();
  _map_grad_phi_face.copyToDeviceNested();
  _map_grad_psi_face.copyToDeviceNested();

  _n_dofs.copyToDevice();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Native implementation helpers
// ─────────────────────────────────────────────────────────────────────────────

#ifdef MOOSE_KOKKOS_NATIVE_FE

// Map a face quadrature point from the side element's reference coordinate
// system to the parent element's reference coordinate system.
//
// parent     — the parent reference element (e.g. reference QUAD9)
// side       — local side index on the parent
// side_topo  — Kokkos topology of the side element
// face_qpt   — quadrature point in the side element's reference system
//
// The mapping is the isoparametric one:
//   x_parent = sum_k  psi_k(face_qpt)  *  master_point_k_in_parent
//
// where psi_k are the LAGRANGE shape functions of the side element.
//
// NOTE: We deliberately avoid Elem::side_ptr() here.  The Quad base class
// marks side_ptr() as 'override final' and it always returns a primitive
// EDGE2, even for second-order quads (QUAD9).  Using that 2-node element
// for the isoparametric mapping silently drops the midpoint contribution,
// yielding wrong parent reference coordinates.  Instead we query the full
// set of side nodes via nodes_on_side() and look up their positions through
// master_point(), which always returns the correct reference coordinates for
// the complete (possibly second-order) side.
static Real3
mapFaceQpToParent(const libMesh::Elem & parent,
                  unsigned int side,
                  FEElemTopology side_topo,
                  Real3 face_qpt)
{
  // Full node list for this side (vertices + midpoints for second-order elems).
  auto side_nodes = parent.nodes_on_side(side);
  const unsigned int n = side_nodes.size();

  Real3 parent_pt;

  // 1-D elements: sides are single vertex nodes.  No reference coordinate
  // on a point element — just return the node's master-element position.
  if (n == 1)
  {
    const auto master = parent.master_point(side_nodes[0]);
    parent_pt.v[0] = master(0);
    parent_pt.v[1] = master(1);
    parent_pt.v[2] = master(2);
    return parent_pt;
  }

  for (unsigned int k = 0; k < n; ++k)
  {
    Real psi;
    const Real s = face_qpt.v[0];
    const Real t = face_qpt.v[1];

    switch (side_topo)
    {
      case FEElemTopology::EDGE2:
        psi = FEEvaluator<LagrangeTag, Edge2Tag>::shape(k, s, 0.0, 0.0);
        break;
      case FEElemTopology::EDGE3:
        psi = FEEvaluator<LagrangeTag, Edge3Tag>::shape(k, s, 0.0, 0.0);
        break;
      case FEElemTopology::TRI3:
        psi = FEEvaluator<LagrangeTag, Tri3Tag>::shape(k, s, t, 0.0);
        break;
      case FEElemTopology::TRI6:
        psi = FEEvaluator<LagrangeTag, Tri6Tag>::shape(k, s, t, 0.0);
        break;
      case FEElemTopology::QUAD4:
        psi = FEEvaluator<LagrangeTag, Quad4Tag>::shape(k, s, t, 0.0);
        break;
      case FEElemTopology::QUAD8:
        psi = FEEvaluator<LagrangeTag, Quad8Tag>::shape(k, s, t, 0.0);
        break;
      case FEElemTopology::QUAD9:
        psi = FEEvaluator<LagrangeTag, Quad9Tag>::shape(k, s, t, 0.0);
        break;
      default:
        psi = 0.0;
        break;
    }

    const auto master = parent.master_point(side_nodes[k]);
    parent_pt.v[0] += psi * master(0);
    parent_pt.v[1] += psi * master(1);
    parent_pt.v[2] += psi * master(2);
  }

  return parent_pt;
}

// Evaluate shape(i, xi, eta, zeta) and grad_shape(i, xi, eta, zeta) for the
// given topology and write results into dest_phi and dest_grad at position
// (i, qp).  Used by both initShapeNative (physics DOFs) and initMapNative
// (geometric map DOFs) to avoid duplicating the big switch.
#define MOOSE_KOKKOS_EVAL_SHAPE(Tag, dest_phi, dest_grad, i, qp, xi, eta, zeta) \
  do                                                                              \
  {                                                                               \
    (dest_phi)(i, qp) = FEEvaluator<LagrangeTag, Tag>::shape(i, xi, eta, zeta); \
    (dest_grad)(i, qp) =                                                         \
        FEEvaluator<LagrangeTag, Tag>::grad_shape(i, xi, eta, zeta);             \
  } while (false)

static void
evalShapeAtQp(FEElemTopology topo,
              unsigned int i,
              unsigned int qp,
              Real xi,
              Real eta,
              Real zeta,
              Array2D<Real> & phi,
              Array2D<Real3> & grad_phi)
{
  switch (topo)
  {
    case FEElemTopology::EDGE2:  MOOSE_KOKKOS_EVAL_SHAPE(Edge2Tag,  phi, grad_phi, i, qp, xi, eta, zeta); break;
    case FEElemTopology::EDGE3:  MOOSE_KOKKOS_EVAL_SHAPE(Edge3Tag,  phi, grad_phi, i, qp, xi, eta, zeta); break;
    case FEElemTopology::TRI3:   MOOSE_KOKKOS_EVAL_SHAPE(Tri3Tag,   phi, grad_phi, i, qp, xi, eta, zeta); break;
    case FEElemTopology::TRI6:   MOOSE_KOKKOS_EVAL_SHAPE(Tri6Tag,   phi, grad_phi, i, qp, xi, eta, zeta); break;
    case FEElemTopology::QUAD4:  MOOSE_KOKKOS_EVAL_SHAPE(Quad4Tag,  phi, grad_phi, i, qp, xi, eta, zeta); break;
    case FEElemTopology::QUAD8:  MOOSE_KOKKOS_EVAL_SHAPE(Quad8Tag,  phi, grad_phi, i, qp, xi, eta, zeta); break;
    case FEElemTopology::QUAD9:  MOOSE_KOKKOS_EVAL_SHAPE(Quad9Tag,  phi, grad_phi, i, qp, xi, eta, zeta); break;
    case FEElemTopology::TET4:   MOOSE_KOKKOS_EVAL_SHAPE(Tet4Tag,   phi, grad_phi, i, qp, xi, eta, zeta); break;
    case FEElemTopology::TET10:  MOOSE_KOKKOS_EVAL_SHAPE(Tet10Tag,  phi, grad_phi, i, qp, xi, eta, zeta); break;
    case FEElemTopology::HEX8:   MOOSE_KOKKOS_EVAL_SHAPE(Hex8Tag,   phi, grad_phi, i, qp, xi, eta, zeta); break;
    case FEElemTopology::HEX20:  MOOSE_KOKKOS_EVAL_SHAPE(Hex20Tag,  phi, grad_phi, i, qp, xi, eta, zeta); break;
    case FEElemTopology::HEX27:  MOOSE_KOKKOS_EVAL_SHAPE(Hex27Tag,  phi, grad_phi, i, qp, xi, eta, zeta); break;
    default: break;
  }
}

#undef MOOSE_KOKKOS_EVAL_SHAPE

// Key-based overload: dispatches on FEShapeKey (LAGRANGE or MONOMIAL).
// Called by initShapeNative for physics DOFs.
static void
evalShapeAtQp(FEShapeKey key,
              unsigned int i,
              unsigned int qp,
              Real xi,
              Real eta,
              Real zeta,
              Array2D<Real> & phi,
              Array2D<Real3> & grad_phi)
{
  phi(i, qp)      = nativeShape(key, i, xi, eta, zeta);
  grad_phi(i, qp) = nativeGradShape(key, i, xi, eta, zeta);
}

void
Assembly::initShapeNative(unsigned int sid,
                           unsigned int etid,
                           unsigned int ftid,
                           FEShapeKey key,
                           FEElemTopology geom_topo,
                           unsigned int n_vol_qps,
                           unsigned int n_face_qps,
                           const libMesh::Elem & ref_elem)
{
  const unsigned int n = nDofs(key);
  _n_dofs(etid, ftid) = n;

  // ── Volume shape functions ────────────────────────────────────────────────

  _phi(sid, etid, ftid).create(n, n_vol_qps);
  _grad_phi(sid, etid, ftid).create(n, n_vol_qps);

  for (unsigned int i = 0; i < n; ++i)
    for (unsigned int qp = 0; qp < n_vol_qps; ++qp)
    {
      const Real xi   = _q_points(sid, etid)[qp].v[0];
      const Real eta  = _q_points(sid, etid)[qp].v[1];
      const Real zeta = _q_points(sid, etid)[qp].v[2];
      evalShapeAtQp(key, i, qp, xi, eta, zeta,
                    _phi(sid, etid, ftid),
                    _grad_phi(sid, etid, ftid));
    }

  // ── Face shape functions ──────────────────────────────────────────────────
  // Face quadrature points (_q_points_face) are in the side element's reference
  // coordinate system.  We use the isoparametric mapping through the side
  // element's nodes (which are stored at parent reference coordinates) to obtain
  // the parent reference coordinates required by FEEvaluator.
  // geom_topo is used for the side topology lookup (geometry-only, not physics).

  const unsigned int n_sides = ref_elem.n_sides();
  const auto side_topo = getSideTopology(geom_topo);

  _phi_face(sid, etid, ftid).create(n_sides);
  _grad_phi_face(sid, etid, ftid).create(n_sides);

  for (unsigned int side = 0; side < n_sides; ++side)
  {
    _phi_face(sid, etid, ftid)(side).create(n, n_face_qps);
    _grad_phi_face(sid, etid, ftid)(side).create(n, n_face_qps);

    for (unsigned int i = 0; i < n; ++i)
      for (unsigned int qp = 0; qp < n_face_qps; ++qp)
      {
        const Real3 par = mapFaceQpToParent(
            ref_elem, side, side_topo, _q_points_face(sid, etid)[side][qp]);
        evalShapeAtQp(key, i, qp, par.v[0], par.v[1], par.v[2],
                      _phi_face(sid, etid, ftid)(side),
                      _grad_phi_face(sid, etid, ftid)(side));
      }
  }
}

void
Assembly::initMapNative(unsigned int sid,
                         unsigned int etid,
                         FEElemTopology topo,
                         unsigned int parent_dim,
                         const libMesh::Elem & ref_elem)
{
  // ── Volume map arrays ────────────────────────────────────────────────────
  // FEMapEvaluator fills _map_phi and _map_grad_phi from the pre-cached volume
  // quadrature points (in parent reference coordinates).

  FEMapEvaluator::fillMapPhi(_map_phi(sid, etid), topo, _q_points(sid, etid));
  FEMapEvaluator::fillMapGradPhi(_map_grad_phi(sid, etid), topo, _q_points(sid, etid));

  // ── Face map arrays ───────────────────────────────────────────────────────

  const unsigned int n_sides = ref_elem.n_sides();
  const auto side_topo = getSideTopology(topo);

  _map_phi_face(sid, etid).create(n_sides);
  _map_grad_phi_face(sid, etid).create(n_sides);
  _map_psi_face(sid, etid).create(n_sides);
  _map_grad_psi_face(sid, etid).create(n_sides);

  // Number of map nodes equals the Lagrange DOFs of the parent topology
  const unsigned int n_map = nDofs(FEFamily::LAGRANGE, topo);

  for (unsigned int side = 0; side < n_sides; ++side)
  {
    const auto & face_qpts = _q_points_face(sid, etid)[side];
    const unsigned int n_face_qps = face_qpts.size();

    // _map_phi_face / _map_grad_phi_face: parent LAGRANGE shapes at face qps
    // expressed in parent reference coordinates.  Requires the same
    // side-to-parent mapping used by initShapeNative.
    _map_phi_face(sid, etid)(side).create(n_map, n_face_qps);
    _map_grad_phi_face(sid, etid)(side).create(n_map, n_face_qps);

    for (unsigned int i = 0; i < n_map; ++i)
      for (unsigned int qp = 0; qp < n_face_qps; ++qp)
      {
        const Real3 par = mapFaceQpToParent(ref_elem, side, side_topo, face_qpts[qp]);
        evalShapeAtQp(topo, i, qp, par.v[0], par.v[1], par.v[2],
                      _map_phi_face(sid, etid)(side),
                      _map_grad_phi_face(sid, etid)(side));
      }

    // _map_psi_face / _map_grad_psi_face: side LAGRANGE shapes at face qps
    // in the side element's own reference coordinate system.
    // FEMapEvaluator handles these directly.
    FEMapEvaluator::fillMapPsiFace(_map_psi_face(sid, etid)(side), side_topo, face_qpts);
    FEMapEvaluator::fillMapGradPsiFace(
        _map_grad_psi_face(sid, etid)(side), side_topo, face_qpts, parent_dim);
  }
}

#endif // MOOSE_KOKKOS_NATIVE_FE

void
Assembly::cachePhysicalMap()
{
  const auto num_subdomains = kokkosMesh().getNumSubdomains();
  const auto num_elems = kokkosMesh().getNumLocalElements();

  _jacobian.create(num_subdomains);
  _jxw.create(num_subdomains);
  _xyz.create(num_subdomains);

  for (auto subdomain : _mesh.meshSubdomains())
  {
    auto sid = kokkosMesh().getContiguousSubdomainID(subdomain);

    _jacobian[sid].createDevice(_n_subdomain_qps[sid]);
    _jxw[sid].createDevice(_n_subdomain_qps[sid]);
    _xyz[sid].createDevice(_n_subdomain_qps[sid]);
  }

  _jacobian.copyToDeviceNested();
  _jxw.copyToDeviceNested();
  _xyz.copyToDeviceNested();

  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>> policy(0, num_elems);
  ::Kokkos::parallel_for(policy, *this);
  ::Kokkos::fence();
}

KOKKOS_FUNCTION void
Assembly::operator()(const ThreadID tid) const
{
  auto info = kokkosMesh().getElementInfo(tid);
  auto offset = getQpOffset(info);

  auto jacobian = &_jacobian[info.subdomain][offset];
  auto jxw = &_jxw[info.subdomain][offset];
  auto xyz = &_xyz[info.subdomain][offset];

  for (unsigned int qp = 0; qp < getNumQps(info); ++qp)
    computePhysicalMap(info, qp, &jacobian[qp], &jxw[qp], &xyz[qp]);
}

} // namespace Moose::Kokkos
