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

#ifndef MTUSEROBJECT_H
#define MTUSEROBJECT_H

#include "GeneralUserObject.h"

class MTUserObject;

template<>
InputParameters validParams<MTUserObject>();


/**
 * Demonstration of user-data object
 */
class MTUserObject : public GeneralUserObject
{
public:
  MTUserObject(const std::string & name, InputParameters params);
  virtual ~MTUserObject();

  /**
   * Called before execute() is ever called so that data can be cleared.
   */
  virtual void initialize(){}

  /**
   * Called when this object needs to compute something.
   */
  virtual void execute() {}

  virtual void finalize() {}

  virtual void destroy();

  /**
   * A function that does something
   */
  Real doSomething() const;

  /**
   * Get scalar value
   */
  Real getScalar() const { return _scalar; }

  /**
   * Set the scalar value
   * @param scalar
   */
  void setScalar(Real scalar) { _scalar = scalar; }

  virtual void load(std::ifstream & stream);
  virtual void store(std::ofstream & stream);

protected:
  /// A scalar value
  Real _scalar;
  /// A vector value
  const std::vector<Real> & _vector;
  /// Dynamically allocated memory
  Real * _dyn_memory;

protected:
  /// Number of elements to allocate (we do not like magic numbers)
  static const unsigned int NUM = 10;
};


#endif /* MTUSEROBJECT_H */
