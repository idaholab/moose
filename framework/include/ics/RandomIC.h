#ifndef RANDOMIC_H
#define RANDOMIC_H

#include "Kernel.h"
#include "InitialCondition.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

// Forward Declarations
class RandomIC;

template<>
InputParameters validParams<RandomIC>();

/**
 * RandomIC just returns a Random value.
 */
class RandomIC : public InitialCondition
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   */
  RandomIC(std::string name,
             InputParameters parameters,
             std::string var_name);

  /**
   * The value of the variable at a point.
   *
   * This must be overriden by derived classes.
   */
  virtual Real value(const Point & p);

private:
  Real _min;
  Real _max;
  Real _range;
};

#endif //RANDOMIC_H
