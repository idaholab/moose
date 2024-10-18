//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
// libMesh
#include "libmesh/enum_order.h"
#include "libmesh/enum_quadrature_type.h"

/**
 * Sets the quadrature
 */
class SetupQuadratureAction : public Action
{
public:
  static InputParameters validParams();

  SetupQuadratureAction(const InputParameters & parameters);

  virtual void act() override;

  /// Return the possible selections for the type of the quadrature
  static MooseEnum getQuadratureTypesEnum()
  {
    return MooseEnum("CLOUGH CONICAL GAUSS GRID MONOMIAL SIMPSON TRAP GAUSS_LOBATTO", "GAUSS");
  }
  /// Return the potential selections for the order of the quadrature, with an 'auto' default
  static MooseEnum getQuadratureOrderEnum()
  {
    return MooseEnum(
        "AUTO CONSTANT FIRST SECOND THIRD FOURTH FIFTH SIXTH SEVENTH EIGHTH NINTH TENTH "
        "ELEVENTH TWELFTH THIRTEENTH FOURTEENTH FIFTEENTH SIXTEENTH SEVENTEENTH "
        "EIGHTTEENTH NINTEENTH TWENTIETH",
        "AUTO");
  }
  /// A MultiMooseEnum for selecting multiple quadrature orders
  static MultiMooseEnum getQuadratureOrdersMultiEnum()
  {
    return MultiMooseEnum(
        "CONSTANT FIRST SECOND THIRD FOURTH FIFTH SIXTH SEVENTH EIGHTH NINTH TENTH "
        "ELEVENTH TWELFTH THIRTEENTH FOURTEENTH FIFTEENTH SIXTEENTH SEVENTEENTH "
        "EIGHTTEENTH NINTEENTH TWENTIETH");
  }

protected:
  libMesh::QuadratureType _type;
  Order _order;
  Order _element_order;
  Order _side_order;
  const std::vector<std::pair<SubdomainID, MooseEnumItem>> _custom_block_orders;
  const bool _allow_negative_qweights;
};
