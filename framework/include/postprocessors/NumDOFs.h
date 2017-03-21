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

#ifndef NUMDOFS_H
#define NUMDOFS_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class NumDOFs;

// libMesh forward declarations
namespace libMesh
{
class System;
class EquationSystems;
}

template <>
InputParameters validParams<NumDOFs>();

class NumDOFs : public GeneralPostprocessor
{
public:
  NumDOFs(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual Real getValue() override;

protected:
  enum SystemEnum
  {
    NL,
    AUX,
    ALL
  };

  const SystemEnum _system_enum;

  const System * _system_pointer;
  const EquationSystems * _es_pointer;
};

#endif // NUMDOFS_H
