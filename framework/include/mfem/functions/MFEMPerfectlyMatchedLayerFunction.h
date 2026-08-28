//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "Function.h"
#include "MFEMBlockRestrictable.h"
#include "MFEMPMLStretchVector.h"

class MFEMProblem;

/**
 * Declares one perfectly matched layer coefficient, to be passed to the curl curl or vector FE mass
 * kernel acting on the layer in place of the plain coefficient of that bilinear form. The layer is
 * the block this function is restricted to.
 *
 * A coefficient covers one choice of scalar or matrix, curl or field tensor, and real or imaginary
 * part, so one function is declared for each combination the problem needs.
 */
class MFEMPerfectlyMatchedLayerFunction : public Function, public MFEMBlockRestrictable
{
public:
  static InputParameters validParams();

  MFEMPerfectlyMatchedLayerFunction(const InputParameters & parameters);

  void initialSetup() override;

protected:
  /// Return the owning MFEM problem, which holds the coefficients declared and read here.
  MFEMProblem & getMFEMProblem() const;

  /// Reserved coefficient name identifying the stretch for this layer and these profile parameters.
  std::string stretchName() const;

  /// The stretch for this layer, declared under stretchName() the first time it is asked for and
  /// shared by every function acting on the same layer thereafter.
  MFEMPMLStretchVector & getStretch();

  /// Stretch shared by every perfectly matched layer function acting on this layer.
  MFEMPMLStretchVector & _stretch_vec;
  /// Base scalar coefficient the stretch scales.
  mfem::Coefficient * _base_coefficient;
};

#endif