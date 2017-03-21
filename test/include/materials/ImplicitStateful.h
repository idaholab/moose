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
#ifndef IMPLICITSTATEFUL_H
#define IMPLICITSTATEFUL_H

#include "Material.h"

// Forward Declarations
class ImplicitStateful;

template <>
InputParameters validParams<ImplicitStateful>();

/**
 * Stateful material class that defines a few properties.
 */
class ImplicitStateful : public Material
{
public:
  ImplicitStateful(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

private:
  bool _add_time;
  bool _use_older;
  MaterialProperty<Real> & _prop;
  const MaterialProperty<Real> & _coupled_old;
  const MaterialProperty<Real> & _coupled_older;
};

#endif // STATEFULMATERIAL_H
