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

#ifndef RANDOMELEMENTALUSEROBJECT_H
#define RANDOMELEMENTALUSEROBJECT_H

#include "ElementUserObject.h"

//Forward Declarations
class RandomElementalUserObject;

template<>
InputParameters validParams<RandomElementalUserObject>();

/**
 * An Elemental user object tha uses built-in Random number generation.
 */
class RandomElementalUserObject : public ElementUserObject
{
public:
  RandomElementalUserObject(const std::string & name, InputParameters parameters);

  virtual ~RandomElementalUserObject();

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize();

  unsigned long getElementalValue(unsigned int element_id) const;

protected:
  std::map<dof_id_type, unsigned long> _random_data;
};

#endif //RANDOMELEMENTALUSEROBJECT_H
