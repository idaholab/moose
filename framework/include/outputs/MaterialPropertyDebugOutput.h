/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MATERIALPROPERTYDEBUGOUTPUT_H
#define MATERIALPROPERTYDEBUGOUTPUT_H

// MOOSE includes
#include "BasicOutput.h"
#include "Output.h"

// Forward declerations
class MaterialPropertyDebugOutput;
class Material;

template <>
InputParameters validParams<MaterialPropertyDebugOutput>();

/**
 * A class for producing various debug related outputs
 *
 * This class may be used from inside the [Outputs] block or via the [Debug] block (preferred)
 */
class MaterialPropertyDebugOutput : public BasicOutput<Output>
{
public:
  /**
   * Class constructor
   * @param parameters Object input parameters
   */
  MaterialPropertyDebugOutput(const InputParameters & parameters);

protected:
  /**
   * Perform the debugging output
   * For this object this is empty; the output is preformed in the constructor
   */
  virtual void output(const ExecFlagType & type) override;

  /**
   * Prints material property information in a format similar to Moose system information
   */
  void printMaterialMap() const;

  /**
   * Builds a output streams for the properties in each material object
   * @param output The output stream to populate
   * @param materials Vector of pointers to the Material objects of interest
   */
  void printMaterialProperties(std::stringstream & output,
                               const std::vector<std::shared_ptr<Material>> & materials) const;
};

#endif // MATERIALPROPERTYEBUGOUTPUT_H
