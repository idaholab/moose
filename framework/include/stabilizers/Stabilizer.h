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

#ifndef STABILIZER_H
#define STABILIZER_H

#include "Moose.h"
#include "PDEBase.h"
#include "MaterialPropertyInterface.h"

// System includes
#include <string>

//forward declarations
class Stabilizer;
class MooseSystem;
class ElementData;

template<>
InputParameters validParams<Stabilizer>();

/**
 * Stabilizers compute modified test function spaces to stabilize oscillating solutions.
 */
class Stabilizer : public PDEBase, protected MaterialPropertyInterface
{
public:
  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param moose_system The reference to the MooseSystem that this object is contained within
   * @param parameters The parameters object holding data for the class to use.
   */
  Stabilizer(const std::string & name, MooseSystem & moose_system, InputParameters parameters);

  virtual ~Stabilizer();

  /**
   * This pure virtual must be overridden by derived classes!
   *
   * This is where the stabilization scheme should compute the test function space.
   * This usually entails multiplying _phi by something and storing it in _test
   */
  virtual void computeTestFunctions() = 0;

protected:
  /**
   * Convenience reference to the ElementData object inside of MooseSystem
   */
  ElementData & _element_data;

  /**
   * Interior test function.
   *
   * These are non-const so they can be modified for stabilization.
   */
  std::vector<std::vector<Real> > & _test;

  /**
   * Gradient of interior test function.
   */
  const std::vector<std::vector<RealGradient> > & _grad_test;

private:

  // Just here to satisfy the pure virtual
  virtual Real computeQpResidual();
};

#endif //STABILIZER_H
