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

/**
 * User object that get a message from an input file and
 * print it out during the simulation
 */
class MessageFromInput : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MessageFromInput(const InputParameters & parameters);
  virtual ~MessageFromInput();

  // Required implementation of a pure virtual function (not used)
  virtual void initialize() override{};

  // Required implementation of a pure virtual function (not used)
  virtual void finalize() override{};

  // Print out the message
  virtual void execute() override;

private:
  // hold the message from the input file
  const std::string _input_message;
};
