//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralUserObject.h"

/* This class is here to combine the Postprocessor interface and the
 * base class Postprocessor object along with adding MooseObject to the inheritance tree*/
class MaterialPropertyUserObject : public ElementIntegralUserObject
{
public:
  static InputParameters validParams();

  MaterialPropertyUserObject(const InputParameters & parameters);

  virtual ~MaterialPropertyUserObject() {}

  virtual void initialize();
  virtual void execute();
  virtual void finalize();
  virtual void threadJoin(const UserObject & y);

  virtual Real computeQpIntegral();

  Real getElementalValue(unsigned int elem_id) const;

protected:
  const MaterialProperty<Real> & _mat_prop;

  std::vector<Real> _elem_integrals;
};
