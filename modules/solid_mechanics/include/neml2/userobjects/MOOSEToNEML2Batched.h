//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MOOSEToNEML2.h"
#include "ElementUserObject.h"

class MOOSEToNEML2Batched : public MOOSEToNEML2, public ElementUserObject
{
public:
  static InputParameters validParams();

  MOOSEToNEML2Batched(const InputParameters & params);

#ifndef NEML2_ENABLED
  void initialize() override {}
  void execute() override {}
  void finalize() override {}
  void threadJoin(const UserObject &) override {}
#else
  void initialize() override;
  void execute() override;
  void finalize() override {}
  void threadJoin(const UserObject &) override;

  neml2::Tensor gatheredData() const override;

  // The number of gathered data items (for setting the model batch size)
  std::size_t size() const { return _buffer.size(); }

protected:
  /// Convert the underlying MOOSE data to a torch::Tensor
  virtual torch::Tensor convertQpMOOSEData() const = 0;

  /// Intermediate data buffer, filled during the element loop
  std::vector<torch::Tensor> _buffer;

  /// Current element's quadrature point indexing
  unsigned int _qp;
#endif
};
