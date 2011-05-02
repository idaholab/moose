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

#ifndef PRESETNODALBC_H
#define PRESETNODALBC_H

#include "NodalBC.h"

class PresetNodalBC;

template<>
InputParameters validParams<PresetNodalBC>();

/**
 * TODO
 */
class PresetNodalBC : public NodalBC
{
public:
  PresetNodalBC(const std::string & name, InputParameters parameters);

  void computeValue(NumericVector<Number> & current_solution);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpValue() = 0;

};

#endif /* PRESETBC_H */
