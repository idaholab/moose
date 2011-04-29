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

#ifndef PRESETBC_H
#define PRESETBC_H

#include "PresetNodalBC.h"

class PresetBC;

template<>
InputParameters validParams<PresetBC>();


class PresetBC : public PresetNodalBC
{
public:
  PresetBC(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpValue();

  Real _value;
};

#endif /* PRESETBC_H */
