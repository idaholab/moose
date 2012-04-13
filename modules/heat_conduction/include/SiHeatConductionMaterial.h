#ifndef SIHEATCONDUCTIONMATERIAL_H
#define SIHEATCONDUCTIONMATERIAL_H

#include "HeatConductionMaterial.h"


//Forward Declarations
class SiHeatConductionMaterial;

template<>
InputParameters validParams<SiHeatConductionMaterial>();

/**
 * Simple material with constant properties.
 */
class SiHeatConductionMaterial : public HeatConductionMaterial
{
public:
  SiHeatConductionMaterial(const std::string & name,
                         InputParameters parameters);

protected:
  virtual void computeProperties();
};

#endif //SIHEATCONDUCTIONMATERIAL_H
