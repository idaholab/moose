//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Output.h"

class MaterialBase;
class MooseObjectName;

/**
 * A class for producing various debug related outputs
 *
 * This class may be used from inside the [Outputs] block or via the [Debug] block (preferred)
 */
class MaterialPropertyDebugOutput : public Output
{
public:
  static InputParameters validParams();
  MaterialPropertyDebugOutput(const InputParameters & parameters);

protected:
  /**
   * Perform the debugging output
   * For this object this is empty; the output is performed in the constructor
   */
  virtual void output(const ExecFlagType & type) override;

  /**
   * Prints material property information in a format similar to Moose system information
   */
  void printMaterialMap() const;

  /**
   * Builds a output streams for the properties in each material object
   * @param output The output stream to populate
   * @param materials Vector of pointers to the MaterialBase objects of interest
   */
  void printMaterialProperties(std::stringstream & output,
                               const std::vector<std::shared_ptr<MaterialBase>> & materials) const;
};
