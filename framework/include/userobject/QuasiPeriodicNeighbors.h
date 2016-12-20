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

#ifndef QUASIPERIODICNEIGHBORS_H
#define QUASIPERIODICNEIGHBORS_H

#include "GeneralUserObject.h"

class QuasiPeriodicNeighbors;
class MooseMesh;

template<>
InputParameters validParams<QuasiPeriodicNeighbors>();

class QuasiPeriodicNeighbors : public GeneralUserObject
{
public:
  QuasiPeriodicNeighbors(const InputParameters & parameters);
  void initialSetup() override;

  void execute() override {}
  void initialize() override {};
  void finalize() override {}

protected:
  virtual void setQuasiPeriodicNeighbors();

  MooseMesh & _mesh;
  const unsigned int _component;
};

#endif /* QUASIPERIODICNEIGHBORS_H */
