//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

namespace libMesh
{
class Elem;
}

struct BndElement
{
  BndElement(Elem * elem, unsigned short int side, BoundaryID bnd_id)
    : _elem(elem), _side(side), _bnd_id(bnd_id)
  {
  }

  /// pointer to the element
  Elem * _elem;
  /// side number
  unsigned short int _side;
  /// boundary id for the node
  BoundaryID _bnd_id;
};

struct ConstBndElement
{
  ConstBndElement(const Elem * elem, unsigned short int side, BoundaryID bnd_id)
    : elem(elem), side(side), bnd_id(bnd_id)
  {
  }

  /// pointer to the element
  const Elem * elem;
  /// side number
  unsigned short int side;
  /// boundary id for the node
  BoundaryID bnd_id;
};
