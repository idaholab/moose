#ifndef STATEFULTEST_H
#define STATEFULTEST_H

#include "Material.h"


//Forward Declarations
class StatefulTest;

template<>
InputParameters validParams<StatefulTest>();

/**
 * Empty material for use in simple applications that don't need material properties.
 */
class StatefulTest : public Material
{
public:
  StatefulTest(const std::string & name,
                InputParameters parameters);
  
protected:
  virtual void computeProperties();

  MaterialProperty<Real> & _thermal_conductivity;
  MaterialProperty<Real> & _thermal_conductivity_old;
};

#endif //STATEFULTEST_H
