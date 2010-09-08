#ifndef EXAMPLEIC_H
#define EXAMPLEIC_H

// MOOSE Includes
#include "Kernel.h"
#include "InitialCondition.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

// Forward Declarations
class ExampleIC;

template<>
InputParameters validParams<ExampleIC>();

/**
 * ExampleIC just returns a constant value.
 */
class ExampleIC : public InitialCondition
{
public:

  /**
   * Constructor: Same as the rest of the MOOSE Objects
   */
  ExampleIC(const std::string & name,
            MooseSystem & moose_system,
            InputParameters parameters);

  /**
   * The value of the variable at a point.
   *
   * This must be overriden by derived classes.
   */
  virtual Real value(const Point & p);

private:
  Real _value;
};

#endif //EXAMPLEIC_H
