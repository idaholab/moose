//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>
#include <unordered_map>

namespace StochasticTools
{

// A base class for a generic neural network, the libotrch and tensorflow version
// will both inherit from this. This class is used to implement functions/members which
// are the same for both.

class NeuralNetBase
{
public:
  // Generic constructor
  NeuralNetBase() {}

  virtual ~NeuralNetBase() {}

  // Virtual function which can be used to add a layer to a neural network.
  // The input will
  virtual void addLayer(std::string layer_name,
                        std::unordered_map<std::string, unsigned int> parameters)
  {
  }
};

}
