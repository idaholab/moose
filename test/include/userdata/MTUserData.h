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

#ifndef MTUSERDATA_H
#define MTUSERDATA_H

#include "UserObject.h"

class MTUserData;

template<>
InputParameters validParams<MTUserData>();


/**
 * Demonstration of user-data object
 */
class MTUserData : public UserObject
{
public:
  MTUserData(const std::string & name, InputParameters params);
  virtual ~MTUserData();

  virtual void destroy();

  /**
   * A function that does something
   */
  Real doSomething() const;

protected:
  /// A scalar value
  const Real & _scalar;
  /// A vector value
  const std::vector<Real> & _vector;
  /// Dynamically allocated memory
  Real * _dyn_memory;

protected:
  /// Number of elements to allocate (we do not like magic numbers)
  static const unsigned int NUM = 10;
};

#endif /* MTUSERDATA_H */
