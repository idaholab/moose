/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MaterialProperty.h"

#include "MooseSystem.h"

template <>
PropertyValue *
MaterialProperty<std::vector<Real> >::init ()
{
  MaterialProperty<std::vector<Real> > *copy = new MaterialProperty<std::vector<Real> >;
  libmesh_assert (copy != NULL);

  copy->_value.resize(_value.size());

  return copy;
}
