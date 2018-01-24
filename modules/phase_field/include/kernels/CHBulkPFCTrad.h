//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef CHBULKPFCTRAD_H
#define CHBULKPFCTRAD_H

#include "CHBulk.h"

// Forward Declarations
class CHBulkPFCTrad;

template <>
InputParameters validParams<CHBulkPFCTrad>();

class CHBulkPFCTrad : public CHBulk<Real>
{
public:
  CHBulkPFCTrad(const InputParameters & parameters);

protected:
  virtual RealGradient computeGradDFDCons(PFFunctionType type);

private:
  const MaterialProperty<Real> & _C0;
  const MaterialProperty<Real> & _a;
  const MaterialProperty<Real> & _b;
};

#endif // CHBULKPFCTRAD_H
