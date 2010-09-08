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

#ifndef SIDEPOSTPROCESSOR_H
#define SIDEPOSTPROCESSOR_H

#include "BoundaryCondition.h"
#include "Postprocessor.h"

//Forward Declarations
class SidePostprocessor;

template<>
InputParameters validParams<SidePostprocessor>();

class SidePostprocessor : public BoundaryCondition, public Postprocessor
{
public:
  SidePostprocessor(const std::string & name, MooseSystem & moose_system, InputParameters parameters);  

  /**
   * TODO: We need to re-evaluate the object model of SidePostprocessors.  Right now this class
   * creates a non-virtual diamond inheritance structure with two separate copies of MooseObject.
   * Both copies contain a complete copy of the parameters object at construction time which at best is
   * just a little wasted memory.  If at some point, a call to add a parameter to one of the parameters
   * object is made the copies will then differ and potentially cause problems when retrieving said
   * parameter later.  For now, I'm explicitly choosing one param object to retrieve from explicitly
   * here so that end user code is consistent and oblivious to the workings underneath.
   */
  template <typename T>
  inline
  const T & getParam(const std::string &name)
  {
    return Postprocessor::getParam<T>(name);
  }

private:
  /**
   * Override the pure virtual... this function should NOT be overridden by other SidePostprocessors
   */
  virtual Real computeQpResidual() { return 0; };
};

#endif
