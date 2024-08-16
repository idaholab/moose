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
NEML2ObjectStubHeader(MOOSEToNEML2Parameter, ElementUserObject);
#else

#include "neml2/tensors/LabeledVector.h"
#include "neml2/tensors/LabeledMatrix.h"
#include "neml2/models/Model.h"

class MOOSEToNEML2Parameter : public ElementUserObject
{
public:
  static InputParameters validParams();

  MOOSEToNEML2Parameter(const InputParameters & params);

  const std::string & getNEML2Parameter() const { return _neml2_parameter; }

  // get the number of gathered data items (for setting the model batch size)
  std::size_t size() const { return _buffer.size(); }

  // inserts the gathered data into the model parameter
  void insertIntoParameter(neml2::Model & model) const;

protected:
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override {}
  virtual void threadJoin(const UserObject &) override;

  /// Convert the underlying MOOSE data to a torch::Tensor
  virtual torch::Tensor convertQpMOOSEData() const = 0;

  /// NEML2 input parameter to transfer data to
  const std::string _neml2_parameter;

  /// Intermediate data buffer, filled during the element loop
  std::vector<torch::Tensor> _buffer;

  /// Current element's quadrature point indexing
  unsigned int _qp;
};

#endif
