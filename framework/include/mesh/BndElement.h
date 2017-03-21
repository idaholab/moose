/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef BNDELEMENT_H
#define BNDELEMENT_H

#include "MooseTypes.h"
#include "libmesh/elem.h"

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

#endif /* BNDELEMENT_H */
