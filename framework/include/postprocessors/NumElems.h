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

#ifndef NUMELEMS_H
#define NUMELEMS_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class NumElems;

template <>
InputParameters validParams<NumElems>();

class NumElems : public GeneralPostprocessor
{
public:
  NumElems(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() override;

private:
  enum class ElemFilter
  {
    ACTIVE,
    TOTAL,
  };
  const ElemFilter _filt;
};

#endif // NUMELEMS_H
