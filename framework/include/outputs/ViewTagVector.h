//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VIEWTAGVECTOR_H
#define VIEWTAGVECTOR_H

#include "PetscOutput.h"

// Forward Declarations
class ViewTagVector;
class NonlinearSystemBase;

template <>
InputParameters validParams<ViewTagVector>();

class ViewTagVector : public PetscOutput
{
public:
  ViewTagVector(const InputParameters & parameters);

  /**
   * Perform the debugging output
   */
  virtual void output(const ExecFlagType & type) override;

protected:
  NonlinearSystemBase & _nl;
  // std::vector<VectorPostprocessorValue> & _vecs;
};

#endif // VIEWTAGVECTOR_H
