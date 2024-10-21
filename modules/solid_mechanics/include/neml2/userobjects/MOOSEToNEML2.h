//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NEML2Utils.h"
#include "ElementUserObject.h"

#ifndef NEML2_ENABLED
NEML2ObjectStubHeader(MOOSEToNEML2, ElementUserObject);
#else

#include "neml2/tensors/LabeledVector.h"
#include "neml2/tensors/LabeledMatrix.h"
#include "neml2/models/Model.h"

class MOOSEToNEML2 : public ElementUserObject
{
public:
  static InputParameters validParams();

  MOOSEToNEML2(const InputParameters & params);

  const neml2::VariableName & getNEML2Variable() const { return _neml2_variable; }

  // get the number of gathered data items (for setting the model batch size)
  std::size_t size() const { return _buffer.size(); }

  // inserts the gathered data into the
  void insertIntoInput(neml2::LabeledVector & input) const;

protected:
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override {}
  virtual void threadJoin(const UserObject &) override;

  /// Convert the underlying MOOSE data to a torch::Tensor
  virtual torch::Tensor convertQpMOOSEData() const = 0;

  /// NEML2 input variable to transfer data to
  const neml2::VariableName _neml2_variable;

  /// Intermediate data buffer, filled during the element loop
  std::vector<torch::Tensor> _buffer;

  /// Current element's quadrature point indexing
  unsigned int _qp;
};

#endif
