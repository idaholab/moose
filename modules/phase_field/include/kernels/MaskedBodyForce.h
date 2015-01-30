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

#ifndef MASKEDBODYFORCE_H
#define MASKEDBODYFORCE_H

#include "BodyForce.h"

//Forward Declarations
class MaskedBodyForce;
class Function;

template<>
InputParameters validParams<MaskedBodyForce>();

/**
 * This kernel creates a body force that is modified by a mask defined
 * as a material. Common uses of this would be to turn off or change the
 * body force in certain regions of the mesh.
 */

class MaskedBodyForce : public BodyForce
{
public:

  MaskedBodyForce(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  std::string _mask_property_name;
  MaterialProperty<Real> & _mask;
};

#endif
