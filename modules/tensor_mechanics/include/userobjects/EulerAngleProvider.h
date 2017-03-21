/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EULERANGLEPROVIDER_H
#define EULERANGLEPROVIDER_H

#include "EulerAngles.h"
#include "GeneralUserObject.h"

class EulerAngleProvider;

template <>
InputParameters validParams<EulerAngleProvider>();

/**
 * Abstract base class for user objects that implement the Euler Angle provider
 * interface.
 */
class EulerAngleProvider : public GeneralUserObject
{
public:
  EulerAngleProvider(const InputParameters & parameters) : GeneralUserObject(parameters) {}

  virtual const EulerAngles & getEulerAngles(unsigned int) const = 0;
  virtual unsigned int getGrainNum() const = 0;
};

#endif // EULERANGLEPROVIDER_H
