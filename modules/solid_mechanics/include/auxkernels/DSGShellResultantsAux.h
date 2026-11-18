//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "libmesh/dense_vector.h"

/**
 * DSGShellResultantsAux writes one stress resultant of a DSG shell element to
 * an auxiliary variable: a membrane force, a bending moment, a transverse shear
 * force or the drilling moment, per unit length and in the element frame.
 */
class DSGShellResultantsAux : public AuxKernel
{
public:
  static InputParameters validParams();

  DSGShellResultantsAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Index of the chosen resultant in the resultants vector of the element
  const unsigned int _resultant_index;

  /// Stress resultants from the resultants material
  const MaterialProperty<DenseVector<Real>> & _resultants;
};
