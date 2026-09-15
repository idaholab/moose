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
#include "LinearSystem.h"
#include "AuxiliarySystem.h"
#include "Assembly.h"
#include "BoundaryRestrictable.h"

#include "libmesh/fe_interface.h"
#include "libmesh/reference_elem.h"

#include <map>
#include <vector>

namespace Moose::Kokkos
{

namespace
{

/**
 * Assemble a reference-space shape function gradient from the per-direction derivatives libMesh
 * reports, which it sizes by dimension so the directions above the mesh dimension are absent
 * @param dphidxi The derivatives with respect to the first reference direction
 * @param dphideta The derivatives with respect to the second reference direction
 * @param dphidzeta The derivatives with respect to the third reference direction
 * @param dimension The mesh dimension
 * @param i The shape function index
 * @param qp The quadrature point index
 * @returns The gradient
 */
Real3
referenceGradient(const std::vector<std::vector<Real>> & dphidxi,
                  const std::vector<std::vector<Real>> & dphideta,
                  const std::vector<std::vector<Real>> & dphidzeta,
                  const unsigned int dimension,
                  const unsigned int i,
                  const unsigned int qp)
{
  Real3 gradient = 0;

  gradient(0) = dphidxi[i][qp];

  if (dimension > 1)
    gradient(1) = dphideta[i][qp];
  if (dimension > 2)
    gradient(2) = dphidzeta[i][qp];

  return gradient;
}

/**
 * Assemble a reference-space vector shape function gradient from the per-direction derivatives
 * libMesh reports, which it sizes by dimension so the directions above the mesh dimension are
 * absent. Row a, column b holds the derivative of component a with respect to reference direction
 * b.
 * @param dphidxi The derivatives with respect to the first reference direction
 * @param dphideta The derivatives with respect to the second reference direction
 * @param dphidzeta The derivatives with respect to the third reference direction
 * @param dimension The mesh dimension
 * @param i The shape function index
 * @param qp The quadrature point index
 * @returns The gradient
 */
Real33
referenceGradient(const std::vector<std::vector<libMesh::RealGradient>> & dphidxi,
                  const std::vector<std::vector<libMesh::RealGradient>> & dphideta,
                  const std::vector<std::vector<libMesh::RealGradient>> & dphidzeta,
                  const unsigned int dimension,
                  const unsigned int i,
                  const unsigned int qp)
{
  Real33 gradient = 0;

  for (const auto component : make_range(3u))
  {
    gradient(component, 0) = dphidxi[i][qp](component);

    if (dimension > 1)
      gradient(component, 1) = dphideta[i][qp](component);
    if (dimension > 2)
      gradient(component, 2) = dphidzeta[i][qp](component);
  }

  return gradient;
}

/**
 * Append the components of a shape value or of a shape function gradient to the key of the row
 * holding it
 * @param key The key to append to
 * @param value The shape value or gradient
 */
///@{
void
appendComponents(std::vector<Real> & key, const Real value)
{
  key.push_back(value);
}

void
appendComponents(std::vector<Real> & key, const Real3 & value)
{
  for (const auto i : make_range(3u))
    key.push_back(value(i));
}

void
appendComponents(std::vector<Real> & key, const Real33 & value)
{
  for (const auto i : make_range(3u))
    for (const auto j : make_range(3u))
      key.push_back(value(i, j));
}
///@}

/**
 * Build the key of the row of a reference shape table that one degree of freedom reads, holding the
 * shape value and its reference gradient at every quadrature point of the table. Degrees of freedom
 * carrying equal keys read equal rows, so a key is what a row of a pool is addressed by.
 * @param phi The shape values
 * @param dphidxi The derivatives with respect to the first reference direction
 * @param dphideta The derivatives with respect to the second reference direction
 * @param dphidzeta The derivatives with respect to the third reference direction
 * @param dimension The mesh dimension
 * @param i The shape function index
 * @param n_points The number of quadrature points of the table
 * @returns The key
 */
template <typename Container>
std::vector<Real>
shapeKey(const Container & phi,
         const Container & dphidxi,
         const Container & dphideta,
         const Container & dphidzeta,
         const unsigned int dimension,
         const unsigned int i,
         const unsigned int n_points)
{
  std::vector<Real> key;

  for (const auto qp : make_range(n_points))
  {
    appendComponents(key, phi[i][qp]);
    appendComponents(key, referenceGradient(dphidxi, dphideta, dphidzeta, dimension, i, qp));
  }

  return key;
}

/**
 * Get the row of a pool that a key is addressed by, adding a row for a key not seen before
 * @param pool The row assigned to each key seen so far
 * @param key The key
 * @returns The row
 */
unsigned int
poolRow(std::map<std::vector<Real>, unsigned int> & pool, const std::vector<Real> & key)
{
  const auto row = pool.size();

  return pool.emplace(key, row).first->second;
}

} // namespace

Assembly::Assembly(FEProblemBase & problem)
  : MeshHolder(*problem.mesh().getKokkosMesh()),
    _problem(problem),
    _mesh(problem.mesh()),
    _dimension(_mesh.dimension())
{
}

void
Assembly::addShapeSystem(libMesh::System & system)
{
  _shape_systems.push_back(&system);
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

      // Cache volume quadrature of each reference element

      qrule->init(*elem, /* p-level */ 0);
      n_qps[subdomain][elem_type] = qrule->n_points();

      _q_points(sid, elem_type_id).create(qrule->n_points());
      _weights(sid, elem_type_id).create(qrule->n_points());

      for (const auto qp : make_range(qrule->n_points()))
      {
        _q_points(sid, elem_type_id)[qp] = qrule->qp(qp);
        _weights(sid, elem_type_id)[qp] = qrule->w(qp);
      }

      // Cache face quadrature of each reference element

      for (const auto side : elem->side_index_range())
      {
        qrule_face->init(*elem->side_ptr(side), /* p-level */ 0);
        n_qps_face[subdomain][elem_type].push_back(qrule_face->n_points());

        _q_points_face(sid, elem_type_id)[side].create(qrule_face->n_points());
        _weights_face(sid, elem_type_id)[side].create(qrule_face->n_points());

        for (const auto qp : make_range(qrule_face->n_points()))
        {
          _q_points_face(sid, elem_type_id)[side][qp] = qrule_face->qp(qp);
          _weights_face(sid, elem_type_id)[side][qp] = qrule_face->w(qp);
        }
      }

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

    for (const auto side : elem->side_index_range())
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

void
Assembly::initShape()
{
  // Generate the list of unique FE types

  std::set<FEType> fe_types;

  auto getFETypes = [&](::System & system)
  {
    for (const auto var : make_range(system.n_vars()))
    {
      const auto fe_type = system.variable_type(var);
      const auto field_type = FEInterface::field_type(fe_type);

      if (field_type == libMesh::TYPE_VECTOR)
      {
        if (fe_type.family != libMesh::LAGRANGE_VEC)
          mooseError("Kokkos currently only supports LAGRANGE_VEC for vector FE families.");
      }
      else
      {
        if (fe_type.family != libMesh::LAGRANGE && fe_type.family != libMesh::L2_LAGRANGE &&
            fe_type.family != libMesh::MONOMIAL && fe_type.family != libMesh::SCALAR &&
            fe_type.family != libMesh::HIERARCHIC)
          mooseError("Kokkos currently only supports LAGRANGE, L2_LAGRNAGE, MONOMIAL, SCALAR, "
                     "HIERARCHIC for scalar FE families.");
      }

      fe_types.insert(fe_type);
    }
  };

  for (const auto nl : make_range(_problem.numNonlinearSystems()))
    getFETypes(_problem.getNonlinearSystemBase(nl).system());

  for (const auto linear : make_range(_problem.numLinearSystems()))
    getFETypes(_problem.getLinearSystem(linear).system());

  getFETypes(_problem.getAuxiliarySystem().system());

  for (auto * const system : _shape_systems)
    getFETypes(*system);

  _fe_type_map.clear();

  for (auto & fet : fe_types)
    _fe_type_map[fet] = _fe_type_map.size();

  // Cache reference shape data

  const auto num_subdomains = kokkosMesh().getNumSubdomains();
  const auto num_elem_types = kokkosMesh().getNumLocalElementTypes();
  const auto num_orientations = kokkosMesh().getMaxNumOrientations();

  _phi.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _phi_face.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _grad_phi.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _grad_phi_face.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _vector_phi.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _vector_phi_face.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _vector_grad_phi.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _vector_grad_phi_face.create(num_subdomains, num_elem_types, _fe_type_map.size());
  _shape_rows.create(num_subdomains, num_elem_types, _fe_type_map.size(), num_orientations);
  _shape_face_rows.create(num_subdomains, num_elem_types, _fe_type_map.size(), num_orientations);
  _is_vector_fe_type.create(_fe_type_map.size());

  _map_phi.create(num_subdomains, num_elem_types);
  _map_phi_face.create(num_subdomains, num_elem_types);
  _map_psi_face.create(num_subdomains, num_elem_types);
  _map_grad_phi.create(num_subdomains, num_elem_types);
  _map_grad_phi_face.create(num_subdomains, num_elem_types);
  _map_grad_psi_face.create(num_subdomains, num_elem_types);

  _normal_dx_dxi.create(num_subdomains, num_elem_types);
  _normal_dx_deta.create(num_subdomains, num_elem_types);

  _n_dofs.create(num_elem_types, _fe_type_map.size());
  _n_dofs = 0;

  for (auto & [fe_type, fe_type_id] : _fe_type_map)
    _is_vector_fe_type[fe_type_id] = FEInterface::field_type(fe_type) == libMesh::TYPE_VECTOR;

  for (auto subdomain : _mesh.meshSubdomains())
  {
    auto sid = kokkosMesh().getContiguousSubdomainID(subdomain);

    auto & assembly = _problem.assembly(0, 0);
    auto qrule = assembly.writeableQRule(_dimension, subdomain, {});
    auto qrule_face = assembly.writeableQRuleFace(_dimension, subdomain, {});

    for (auto & [fe_type, fe_type_id] : _fe_type_map)
    {
      const bool is_vector_fe = FEInterface::field_type(fe_type) == libMesh::TYPE_VECTOR;

      std::unique_ptr<FEBase> fe;
      std::unique_ptr<FEBase> fe_face;
      std::unique_ptr<FEVectorBase> vector_fe;
      std::unique_ptr<FEVectorBase> vector_fe_face;

      if (is_vector_fe)
      {
        vector_fe = FEVectorBase::build(_dimension, fe_type);
        vector_fe_face = FEVectorBase::build(_dimension, fe_type);

        vector_fe->attach_quadrature_rule(qrule);
        vector_fe_face->attach_quadrature_rule(qrule_face);
      }
      else
      {
        fe = FEBase::build(_dimension, fe_type);
        fe_face = FEBase::build(_dimension, fe_type);

        fe->attach_quadrature_rule(qrule);
        fe_face->attach_quadrature_rule(qrule_face);
      }

      for (auto & [elem_type, elem_type_id] : kokkosMesh().getElementTypeMap())
      {
        const auto num_elem_orientations = kokkosMesh().getNumOrientations(elem_type_id);

        // Copies of the type IDs, which a lambda cannot capture out of a structured binding
        const auto elem_id = elem_type_id;
        const auto fe_id = fe_type_id;

        // The shape functions of an FE family may depend on the orientation of the element's edges
        // and faces, so a table is built for each orientation the local mesh presents, evaluated on
        // an element of the mesh that carries it. Only reference-space quantities are read from
        // that element, which keeps the tables independent of its geometry.
        //
        // The row of a table that a degree of freedom reads is addressed by the shape value and
        // reference gradient it holds at every quadrature point, so the orientations that agree on
        // a shape function share one row of a pool, and an orientation carries a list of pool rows.
        // The number of distinct rows then grows with the number of edges and faces of an element
        // that a family orients, rather than with the number of orientations of the whole element,
        // which is what keeps the tables of a high order family affordable. The volume table and
        // the table of each side are addressed separately, each from its own reinit.
        //
        // Filling a pool takes two sweeps over the orientations: the first assigns the rows and so
        // counts them, and the second, once the pool is allocated, fills each row from the first
        // orientation that reads it.

        // The row assigned to each key of the volume pool
        std::map<std::vector<Real>, unsigned int> pool_keys;
        // The row assigned to each key of the pool of each side
        std::vector<std::map<std::vector<Real>, unsigned int>> face_pool_keys;

        auto assign_rows = [&](auto & fe, auto & fe_face)
        {
          auto & phi = fe.get_phi();
          auto & dphidxi = fe.get_dphidxi();
          auto & dphideta = fe.get_dphideta();
          auto & dphidzeta = fe.get_dphidzeta();

          auto & phi_face = fe_face.get_phi();
          auto & dphidxi_face = fe_face.get_dphidxi();
          auto & dphideta_face = fe_face.get_dphideta();
          auto & dphidzeta_face = fe_face.get_dphidzeta();

          for (const auto orientation : make_range(num_elem_orientations))
          {
            auto elem = &kokkosMesh().getOrientationElem(elem_id, orientation);

            fe.reinit(elem);

            _n_dofs(elem_id, fe_id) = phi.size();

            auto & rows = _shape_rows(sid, elem_id, fe_id, orientation);

            rows.create(phi.size());

            for (const auto i : index_range(phi))
              rows(i) = poolRow(
                  pool_keys,
                  shapeKey(phi, dphidxi, dphideta, dphidzeta, _dimension, i, qrule->n_points()));

            auto & face_rows = _shape_face_rows(sid, elem_id, fe_id, orientation);

            face_rows.create(elem->n_sides());
            face_pool_keys.resize(elem->n_sides());

            for (const auto side : elem->side_index_range())
            {
              fe_face.reinit(elem, side);

              face_rows(side).create(phi_face.size());

              for (const auto i : index_range(phi_face))
                face_rows(side)(i) = poolRow(face_pool_keys[side],
                                             shapeKey(phi_face,
                                                      dphidxi_face,
                                                      dphideta_face,
                                                      dphidzeta_face,
                                                      _dimension,
                                                      i,
                                                      qrule_face->n_points()));
            }
          }
        };

        auto fill_pools = [&](auto & fe,
                              auto & fe_face,
                              auto & phi_pool,
                              auto & grad_phi_pool,
                              auto & phi_face_pool,
                              auto & grad_phi_face_pool)
        {
          auto & phi = fe.get_phi();
          auto & dphidxi = fe.get_dphidxi();
          auto & dphideta = fe.get_dphideta();
          auto & dphidzeta = fe.get_dphidzeta();

          auto & phi_face = fe_face.get_phi();
          auto & dphidxi_face = fe_face.get_dphidxi();
          auto & dphideta_face = fe_face.get_dphideta();
          auto & dphidzeta_face = fe_face.get_dphidzeta();

          phi_pool.create(pool_keys.size(), qrule->n_points());
          grad_phi_pool.create(pool_keys.size(), qrule->n_points());

          phi_face_pool.create(face_pool_keys.size());
          grad_phi_face_pool.create(face_pool_keys.size());

          for (const auto side : index_range(face_pool_keys))
          {
            phi_face_pool(side).create(face_pool_keys[side].size(), qrule_face->n_points());
            grad_phi_face_pool(side).create(face_pool_keys[side].size(), qrule_face->n_points());
          }

          // Whether a row holds its shape data yet, so that each row is filled once
          std::vector<bool> filled(pool_keys.size(), false);
          std::vector<std::vector<bool>> face_filled;

          for (const auto side : index_range(face_pool_keys))
            face_filled.emplace_back(face_pool_keys[side].size(), false);

          for (const auto orientation : make_range(num_elem_orientations))
          {
            auto elem = &kokkosMesh().getOrientationElem(elem_id, orientation);

            fe.reinit(elem);

            const auto & rows = _shape_rows(sid, elem_id, fe_id, orientation);

            for (const auto i : index_range(phi))
            {
              const auto row = rows(i);

              if (filled[row])
                continue;

              filled[row] = true;

              for (const auto qp : make_range(qrule->n_points()))
              {
                phi_pool(row, qp) = phi[i][qp];
                grad_phi_pool(row, qp) =
                    referenceGradient(dphidxi, dphideta, dphidzeta, _dimension, i, qp);
              }
            }

            const auto & face_rows = _shape_face_rows(sid, elem_id, fe_id, orientation);

            for (const auto side : elem->side_index_range())
            {
              fe_face.reinit(elem, side);

              for (const auto i : index_range(phi_face))
              {
                const auto row = face_rows(side)(i);

                if (face_filled[side][row])
                  continue;

                face_filled[side][row] = true;

                for (const auto qp : make_range(qrule_face->n_points()))
                {
                  phi_face_pool(side)(row, qp) = phi_face[i][qp];
                  grad_phi_face_pool(side)(row, qp) = referenceGradient(
                      dphidxi_face, dphideta_face, dphidzeta_face, _dimension, i, qp);
                }
              }
            }
          }
        };

        if (is_vector_fe)
        {
          assign_rows(*vector_fe, *vector_fe_face);
          fill_pools(*vector_fe,
                     *vector_fe_face,
                     _vector_phi(sid, elem_id, fe_id),
                     _vector_grad_phi(sid, elem_id, fe_id),
                     _vector_phi_face(sid, elem_id, fe_id),
                     _vector_grad_phi_face(sid, elem_id, fe_id));
        }
        else
        {
          assign_rows(*fe, *fe_face);
          fill_pools(*fe,
                     *fe_face,
                     _phi(sid, elem_id, fe_id),
                     _grad_phi(sid, elem_id, fe_id),
                     _phi_face(sid, elem_id, fe_id),
                     _grad_phi_face(sid, elem_id, fe_id));
        }
      }
    }

    for (auto & [elem_type, elem_type_id] : kokkosMesh().getElementTypeMap())
    {
      auto elem = &libMesh::ReferenceElem::get(elem_type);
      auto fe_type = FEType(elem->default_order(), LAGRANGE);
      auto shape_deriv_ptr = libMesh::FEInterface::shape_deriv_function(fe_type, elem);

      std::unique_ptr<FEBase> fe(FEBase::build(_dimension, fe_type));
      std::unique_ptr<FEBase> fe_face(FEBase::build(_dimension, fe_type));

      fe->attach_quadrature_rule(qrule);
      fe_face->attach_quadrature_rule(qrule_face);

      auto & phi = fe->get_phi();
      auto & grad_phi = fe->get_dphi();

      fe->reinit(elem);

      _map_phi(sid, elem_type_id).create(phi.size(), qrule->n_points());
      _map_grad_phi(sid, elem_type_id).create(phi.size(), qrule->n_points());

      for (const auto i : index_range(phi))
        for (const auto qp : make_range(qrule->n_points()))
        {
          _map_phi(sid, elem_type_id)(i, qp) = phi[i][qp];
          _map_grad_phi(sid, elem_type_id)(i, qp) = grad_phi[i][qp];
        }

      _map_phi_face(sid, elem_type_id).create(elem->n_sides());
      _map_grad_phi_face(sid, elem_type_id).create(elem->n_sides());
      _map_psi_face(sid, elem_type_id).create(elem->n_sides());
      _map_grad_psi_face(sid, elem_type_id).create(elem->n_sides());

      _normal_dx_dxi(sid, elem_type_id).create(elem->n_sides());
      _normal_dx_deta(sid, elem_type_id).create(elem->n_sides());

      for (const auto side : elem->side_index_range())
      {
        auto side_ptr = elem->build_side_ptr(side);

        auto & phi = fe_face->get_phi();
        auto & grad_phi = fe_face->get_dphi();

        fe_face->reinit(elem, side);

        _map_phi_face(sid, elem_type_id)(side).create(phi.size(), qrule_face->n_points());
        _map_grad_phi_face(sid, elem_type_id)(side).create(phi.size(), qrule_face->n_points());

        for (const auto i : index_range(phi))
          for (const auto qp : make_range(qrule_face->n_points()))
          {
            _map_phi_face(sid, elem_type_id)(side)(i, qp) = phi[i][qp];
            _map_grad_phi_face(sid, elem_type_id)(side)(i, qp) = grad_phi[i][qp];
          }

        auto & psi = fe_face->get_fe_map().get_psi();
        auto & dpsidxi = fe_face->get_fe_map().get_dpsidxi();
        auto & dpsideta = fe_face->get_fe_map().get_dpsideta();

        _map_psi_face(sid, elem_type_id)(side).create(psi.size(), qrule_face->n_points());
        _map_grad_psi_face(sid, elem_type_id)(side).create(psi.size(), qrule_face->n_points());

        for (const auto i : index_range(psi))
          for (const auto qp : make_range(qrule_face->n_points()))
          {
            _map_psi_face(sid, elem_type_id)(side)(i, qp) = psi[i][qp];
            if (elem->dim() > 1)
              _map_grad_psi_face(sid, elem_type_id)(side)(i, qp)(0) = dpsidxi[i][qp];
            if (elem->dim() > 2)
              _map_grad_psi_face(sid, elem_type_id)(side)(i, qp)(1) = dpsideta[i][qp];
          }

        _normal_dx_dxi(sid, elem_type_id)(side).create(phi.size(), qrule_face->n_points());
        _normal_dx_deta(sid, elem_type_id)(side).create(phi.size(), qrule_face->n_points());

        for (const auto qp : make_range(qrule_face->n_points()))
        {
          Point reference_point;

          if (_dimension == 1)
            reference_point = side ? Point(1) : Point(-1);
          else if (_dimension == 2)
            for (const auto i : index_range(psi))
              reference_point.add_scaled(side_ptr->point(i), psi[i][qp]);

          for (const auto i : index_range(phi))
          {
            if (_dimension < 3)
              _normal_dx_dxi(sid, elem_type_id)(side)(i, qp) =
                  shape_deriv_ptr(fe_type, elem, i, 0, reference_point, false);
            if (_dimension == 2)
              _normal_dx_deta(sid, elem_type_id)(side)(i, qp) =
                  shape_deriv_ptr(fe_type, elem, i, 1, reference_point, false);
          }
        }
      }
    }
  }

  _phi.copyToDeviceNested();
  _phi_face.copyToDeviceNested();
  _grad_phi.copyToDeviceNested();
  _grad_phi_face.copyToDeviceNested();
  _vector_phi.copyToDeviceNested();
  _vector_phi_face.copyToDeviceNested();
  _vector_grad_phi.copyToDeviceNested();
  _vector_grad_phi_face.copyToDeviceNested();
  _shape_rows.copyToDeviceNested();
  _shape_face_rows.copyToDeviceNested();
  _is_vector_fe_type.copyToDevice();

  _map_phi.copyToDeviceNested();
  _map_phi_face.copyToDeviceNested();
  _map_psi_face.copyToDeviceNested();
  _map_grad_phi.copyToDeviceNested();
  _map_grad_phi_face.copyToDeviceNested();
  _map_grad_psi_face.copyToDeviceNested();

  _normal_dx_dxi.copyToDeviceNested();
  _normal_dx_deta.copyToDeviceNested();

  _n_dofs.copyToDevice();
}

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
