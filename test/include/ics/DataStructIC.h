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
#ifndef DATASTRUCTIC_H
#define DATASTRUCTIC_H

#include "InitialCondition.h"
#include "MooseMesh.h"

class DataStructIC;

template<>
InputParameters validParams<DataStructIC>();

/**
 * This initial condition builds a data structure that is queried
 * for initial condition information
 */
class DataStructIC : public InitialCondition
{
public:
  DataStructIC(const std::string & name, InputParameters parameters);
  virtual ~DataStructIC();

  virtual void initialSetup();

  virtual Real value(const Point & /*p*/);

private:
  MooseMesh & _mesh;
  std::map<dof_id_type, Real> _data;
};


#endif /* DATASTRUCTIC_H */
