//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedElementLoop.h"

#include "libmesh/elem_range.h"

class AuxiliarySystem;
class Adaptivity;
class DisplacedProblem;

class FlagElementsThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  FlagElementsThread(FEProblemBase & fe_problem,
                     std::vector<Number> & serialized_solution,
                     unsigned int max_h_level,
                     const std::string & marker_name,
                     bool is_serialized_solution);

  // Splitting Constructor
  FlagElementsThread(FlagElementsThread & x, Threads::split split);

  virtual void onElement(const Elem * elem) override;

  // Must override this in order to avoid calling the base class method which reads the refinement
  // flags, causing read-write data races
  bool shouldComputeInternalSide(const Elem &, const Elem &) const override { return false; }

  void join(const FlagElementsThread & /*y*/);

protected:
  FEProblemBase & _fe_problem;
  std::shared_ptr<DisplacedProblem> _displaced_problem;
  AuxiliarySystem & _aux_sys;
  unsigned int _system_number;
  Adaptivity & _adaptivity;
  MooseVariableFEBase & _field_var;
  unsigned int _field_var_number;
  std::vector<Number> & _serialized_solution;
  unsigned int _max_h_level;
  bool _is_serialized_solution;
};
