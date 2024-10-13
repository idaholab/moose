//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadraturePointsPositions.h"
#include "libmesh/quadrature_gauss.h"
#include "SetupQuadratureAction.h"

using namespace libMesh;

registerMooseObject("MooseApp", QuadraturePointsPositions);

InputParameters
QuadraturePointsPositions::validParams()
{
  InputParameters params = Positions::validParams();
  params.addClassDescription("Positions of element quadrature points.");
  params += BlockRestrictable::validParams();

  params.addParam<MooseEnum>("quadrature_type",
                             SetupQuadratureAction::getQuadratureTypesEnum(),
                             "Type of the quadrature rule");
  params.addParam<MooseEnum>("quadrature_order",
                             SetupQuadratureAction::getQuadratureOrderEnum(),
                             "Order of the volumetric quadrature. If unspecified, defaults to the "
                             "local element default order. This is not the problem default, which "
                             "is based on the order of the variables in the system.");

  // Element centroids could be sorted by XYZ or by id. Default to not sorting
  params.set<bool>("auto_sort") = false;
  // Gathered locally, should be broadcast on every process
  params.set<bool>("auto_broadcast") = true;

  return params;
}

QuadraturePointsPositions::QuadraturePointsPositions(const InputParameters & parameters)
  : Positions(parameters),
    BlockRestrictable(this),
    _mesh(_fe_problem.mesh()),
    _q_type(Moose::stringToEnum<QuadratureType>(getParam<MooseEnum>("quadrature_type"))),
    _q_order(Moose::stringToEnum<Order>(getParam<MooseEnum>("quadrature_order")))
{
  // Mesh is ready at construction
  initialize();
  // Trigger synchronization as the initialization is distributed
  finalize();
}

void
QuadraturePointsPositions::initialize()
{
  clearPositions();

  // By default, initialize should be called on meshChanged()
  // Gathering of positions is local, reporter system makes sure to make it global
  if (blockRestricted())
  {
    _positions_2d.resize(numBlocks());
    unsigned int b_index = 0;
    for (const auto & sub_id : blockIDs())
    {
      for (const auto & elem : _mesh.getMesh().active_local_subdomain_elements_ptr_range(sub_id))
      {
        // Get a quadrature going of the requested type and order
        const FEFamily mapping_family = FEMap::map_fe_type(*elem);
        const FEType fe_type(elem->default_order(), mapping_family);
        std::unique_ptr<FEBase> fe = FEBase::build(elem->dim(), fe_type);
        const auto q_order =
            (_q_order == INVALID_ORDER) ? fe_type.default_quadrature_order() : _q_order;
        auto qrule = QBase::build(_q_type, elem->dim(), q_order);
        fe->attach_quadrature_rule(qrule.get());
        const auto & q_points = fe->get_xyz();
        fe->reinit(elem);

        for (const auto & q : q_points)
        {
          _positions.emplace_back(q);
          _positions_2d[b_index].emplace_back(q);
        }
      }
      b_index += 1;
    }
  }
  else
  {
    _positions.resize(_mesh.getMesh().n_active_local_elem());
    for (const auto & elem : _mesh.getMesh().active_local_element_ptr_range())
    {
      // Get a quadrature going on the element
      const FEFamily mapping_family = FEMap::map_fe_type(*elem);
      const FEType fe_type(elem->default_order(), mapping_family);
      std::unique_ptr<FEBase> fe = FEBase::build(elem->dim(), fe_type);
      const auto q_order =
          (_q_order == INVALID_ORDER) ? fe_type.default_quadrature_order() : _q_order;
      auto qrule = QBase::build(_q_type, elem->dim(), q_order);
      fe->attach_quadrature_rule(qrule.get());
      const auto & q_points = fe->get_xyz();
      fe->reinit(elem);

      for (const auto & q : q_points)
        _positions.emplace_back(q);
    }
  }
  _initialized = true;
}
