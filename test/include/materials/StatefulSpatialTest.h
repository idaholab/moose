#ifndef STATEFULSPATIALTEST_H
#define STATEFULSPATIALTEST_H

#include "Material.h"


//Forward Declarations
class StatefulSpatialTest;

template<>
InputParameters validParams<StatefulSpatialTest>();

/**
 * Empty material for use in simple applications that don't need material properties.
 */
class StatefulSpatialTest : public Material
{
public:
  StatefulSpatialTest(const std::string & name,
                      InputParameters parameters);

protected:
  virtual void computeQpProperties();
  virtual void initQpStatefulProperties();

  MaterialProperty<Real> & _thermal_conductivity;
  MaterialProperty<Real> & _thermal_conductivity_old;
};

#endif //STATEFULSPATIALTEST_H
