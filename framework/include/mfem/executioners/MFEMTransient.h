//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#pragma once
#include "MFEMExecutioner.h"
#include "TimeDomainProblemOperator.h"

class MFEMTransient : public MFEMExecutioner
{
public:
  static InputParameters validParams();

  explicit MFEMTransient(const InputParameters & params);

  void constructProblemOperator() override;
  void step(double dt, int it) const;
  virtual void init() override;
  virtual void execute() override;

  mutable double _t_step; // Time step

private:
  double _t_initial;       // Start time
  double _t_final;         // End time
  Real & _t;               // Current time
  mutable int _it;         // Time index
  int _vis_steps;          // Number of cycles between each output update
  mutable bool _last_step; // Flag to check if current step is final
  std::unique_ptr<Moose::MFEM::TimeDomainProblemOperator> _problem_operator{nullptr};
};

#endif
