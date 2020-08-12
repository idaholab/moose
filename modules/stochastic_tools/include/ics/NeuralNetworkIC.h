//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"
#include "NeuralNetworkUserObject.h"

/**
 * Initializes variable value by evaluating a NeuralNetworkUserObject on the input variable values
 */
class NeuralNetworkIC : public InitialCondition
{
public:
  static InputParameters validParams();

  NeuralNetworkIC(const InputParameters & parameters);

  // Collects the input variable values on the nodal point and calls the evaluate(DenseVector<Real>
  // & input, std::size_t op_idx) function on the NeuralNetworkUserObject
  virtual Real value(const Point & p);
  virtual const std::set<std::string> & getRequestedItems() override;

protected:
  const NeuralNetworkUserObject & _nn_obj;
  std::vector<const VariableValue *> _input_vect;
  unsigned int _n_inputs;
  std::size_t _op_id;
};
