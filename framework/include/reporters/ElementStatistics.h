//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementReporter.h"

class ElementStatistics : public ElementReporter
{
public:
  static InputParameters validParams();

  ElementStatistics(const InputParameters & parameters);

protected:
  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject &) override;
  virtual void finalize() override;

  virtual Real computeValue() = 0;

private:
  const std::string _base_name;
  Real & _max;
  Real & _min;
  Real & _average;
  Real & _integral;
  int & _number_elements;
};
