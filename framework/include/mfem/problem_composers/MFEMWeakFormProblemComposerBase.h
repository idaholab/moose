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

#include "MFEMProblemComposer.h"

/**
 * Base class for MFEMProblemComposers whose problem operator solves an EquationSystem built by an
 * MFEMWeakFormBase derived object. Composers that build operators with no associated weak form
 * should derive from MFEMProblemComposer directly.
 */
class MFEMWeakFormProblemComposerBase : public MFEMProblemComposer
{
public:
  static InputParameters validParams();

  MFEMWeakFormProblemComposerBase(const InputParameters & parameters);

protected:
  /// Name of the weak form whose EquationSystem the created operator solves. Empty if unset by
  /// the user, in which case the problem's sole EquationSystem is used.
  const MFEMWeakFormName _weak_form_name;
};

#endif
