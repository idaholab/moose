/****************************************************************/
/*               Do NOT MODIFY THIS HEADER                      */
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

#ifndef MATERIALPROPERTYUSEROBJECT_H
#define MATERIALPROPERTYUSEROBJECT_H

#include "ElementIntegralUserObject.h"

//Forward Declarations
class MaterialPropertyUserObject;

template<>
InputParameters validParams<MaterialPropertyUserObject>();

/* This class is here to combine the Postprocessor interface and the
 * base class Postprocessor object along with adding MooseObject to the inheritance tree*/
class MaterialPropertyUserObject :
  public ElementIntegralUserObject
{
public:
  MaterialPropertyUserObject(const std::string & name, InputParameters parameters);

  virtual ~MaterialPropertyUserObject() {}

  virtual void initialize();
  virtual void execute();
  virtual void finalize();
  virtual void threadJoin(const UserObject & y);

  virtual Real computeQpIntegral();

  Real getElementalValue(unsigned int elem_id) const;

protected:

  MaterialProperty<Real> & _mat_prop;

  std::vector<Real> _elem_integrals;
};

#endif
