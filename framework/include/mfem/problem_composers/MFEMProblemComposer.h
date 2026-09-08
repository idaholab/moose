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

#include "MFEMObject.h"

namespace Moose::MFEM
{
class ProblemOperatorBase;
}

/**
 * Interface required for all MFEMProblemComposers
 */
class MFEMProblemComposer : public MFEMObject
{
public:
  static InputParameters validParams();

  MFEMProblemComposer(const InputParameters & parameters);

  virtual ~MFEMProblemComposer() = default;

  /// Returns a pointer to a freshly minted problem operator.
  virtual std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
  createProblemOperator(MFEMProblem &) = 0;

protected:
  /// Name of the weak form whose EquationSystem the created operator solves. Empty if unset by
  /// the user, in which case the problem's sole EquationSystem is used.
  const MFEMWeakFormName _weak_form_name;
};

#endif
