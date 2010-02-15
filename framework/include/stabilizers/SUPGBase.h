#ifndef SUPGBASE_H
#define SUPGBASE_H

#include "Moose.h"
#include "Stabilizer.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

//Forward Declarations
class SUPGBase;

template<>
InputParameters validParams<SUPGBase>();

/**
 * 
 */
class SUPGBase : public Stabilizer
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   * @param coupled_to The real names of the variables this SUPGBase is coupled to.
   * @param coupled_as The name this SUPGBase is going to use to refer to the coupled_to variables as.
   */
  SUPGBase(std::string name,
             InputParameters parameters,
             std::string var_name,
             std::vector<std::string> coupled_to,
             std::vector<std::string> coupled_as);

  /**
   * Compute the test functions.
   */
  virtual void computeTestFunctions();

protected:

  /**
   * This function MUST be overriden!
   *
   * Override it to fill the Tau and Velocity vectors for
   * each quadrature point.  Note that this means you MUST
   * do your own quadrature loop!
   */
  virtual void computeTausAndVelocities() = 0;
  
  /**
   * The velocity at each quadrature point.
   */
  std::vector<RealVectorValue> _velocity;

  /**
   * The Taus at each quadrature point.
   */
  std::vector<Real> _tau;
};

#endif //SUPGBASE_H
