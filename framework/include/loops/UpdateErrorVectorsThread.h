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

class UpdateErrorVectorsThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  UpdateErrorVectorsThread(
      FEProblemBase & fe_problem,
      const std::map<std::string, std::unique_ptr<ErrorVector>> & indicator_field_to_error_vector);

  // Splitting Constructor
  UpdateErrorVectorsThread(UpdateErrorVectorsThread & x, Threads::split split);

  virtual void onElement(const Elem * elem) override;

  void join(const UpdateErrorVectorsThread & /*y*/);

protected:
  const std::map<std::string, std::unique_ptr<ErrorVector>> & _indicator_field_to_error_vector;
  AuxiliarySystem & _aux_sys;
  unsigned int _system_number;
  Adaptivity & _adaptivity;
  NumericVector<Number> & _solution;

  std::map<unsigned int, ErrorVector *> _indicator_field_number_to_error_vector;
};
