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

#ifndef BLOCKAVERAGEVALUE_H
#define BLOCKAVERAGEVALUE_H

#include "ElementIntegralVariablePostprocessor.h"

// libmesh includes
#include "mesh_tools.h"

//Forward Declarations
class BlockAverageValue;

template<>
InputParameters validParams<BlockAverageValue>();

/**
 * Computes the average value of a variable on each block
 */
class BlockAverageValue : public ElementIntegralVariablePostprocessor
{
public:
  BlockAverageValue(const std::string & name, InputParameters parameters);

  /**
   * Given a block ID return the average value for a variable on that block
   *
   * Note that accessor functions on UserObjects like this _must_ be const.
   * That is because the UserObject system returns const references to objects
   * trying to use UserObjects.  This is done for parallel correctness.
   *
   * @return The average value of a variable on that block.
   */
  Real averageValue(SubdomainID block) const;

  /**
   * This is called before execute so you can reset any internal data.
   */
  virtual void initialize();

  /**
   * Called on every "object" (like every element or node).
   * In this case, it is called at every quadrature point on every element.
   */
  virtual void execute();

  /**
   * Called when using threading.  You need to combine the data from "y"
   * into _this_ object.
   */
  virtual void threadJoin(const UserObject & y);

  /**
   * Called _once_ after execute has been called all all "objects".
   */
  virtual void finalize();

protected:
  // This map will hold the partial sums for each block
  std::map<SubdomainID, Real> _integral_values;

  // This map will hold the partial volume sums for each block
  std::map<SubdomainID, Real> _volume_values;

  // This map will hold our averages for each block
  std::map<SubdomainID, Real> _average_values;
};

#endif
