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

#ifndef RESTARTABLETYPESCHECKER_H
#define RESTARTABLETYPESCHECKER_H

#include "GeneralUserObject.h"

class RestartableTypesChecker;

template<>
InputParameters validParams<RestartableTypesChecker>();


/**
 * User Object for testing Restartable data types
 */
class RestartableTypesChecker : public GeneralUserObject
{
public:
  RestartableTypesChecker(const std::string & name, InputParameters params);
  virtual ~RestartableTypesChecker();

  virtual void initialSetup();
  virtual void timestepSetup();

  virtual void initialize() {};
  virtual void execute();
  virtual void finalize() {};

protected:
  bool _first;

  Real & _real_data;
  std::vector<Real> & _vector_data;
  std::vector<std::vector<Real> > & _vector_vector_data;
};


#endif /* RESTARTABLETYPESCHECKER_H */
