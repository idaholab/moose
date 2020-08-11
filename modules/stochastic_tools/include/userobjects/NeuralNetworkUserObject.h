//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "GeneralUserObject.h"
#include "MooseVariableInterface.h"

// forward declarations
class NeuralNetworkUserObject;

template <>
InputParameters validParams<GeneralUserObject>();

class NeuralNetworkUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  NeuralNetworkUserObject(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}
  Real evaluate(DenseVector<Real> & input, std::size_t op_idx) const;

protected:
  void setWeights();
  void setXMLWeights();
  unsigned int _h;
  unsigned int _d_in;
  unsigned int _d_out;
  unsigned int _n;

  FileName _weightsFile;
  std::vector<DenseMatrix<Real>> _weights;
  std::vector<DenseVector<Real>> _bias;

  MultiMooseEnum _layerActivationFunctionEnum;

};
