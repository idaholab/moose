#ifndef COMPUTINGINITIALTEST_H
#define COMPUTINGINITIALTEST_H

#include "Material.h"


//Forward Declarations
class ComputingInitialTest;

template<>
InputParameters validParams<ComputingInitialTest>();

/**
 * Empty material for use in simple applications that don't need material properties.
 */
class ComputingInitialTest : public Material
{
public:
  ComputingInitialTest(const std::string & name,
                InputParameters parameters);

protected:
  virtual void initQpComputingInitialProperties();
  virtual void computeQpProperties();

  MaterialProperty<Real> & _thermal_conductivity;
  MaterialProperty<Real> & _thermal_conductivity_old;
};

#endif //COMPUTINGINITIALTEST_H
