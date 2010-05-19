#ifndef PLASTICMATERIAL_H
#define PLASTICMATERIAL_H

#include "LinearIsotropicMaterial.h"

//Forward Declarations
class PlasticMaterial;

template<>
InputParameters validParams<PlasticMaterial>();

/**
 * Plastic material for use in simple applications that don't need material properties.
 */
class PlasticMaterial : public LinearIsotropicMaterial
{
public:
  PlasticMaterial(std::string name,
                  MooseSystem & moose_system,
                  InputParameters parameters);
  
protected:
  /**
   * Will always be passed to full symmetric strain tensor.
   * What should come out is a modified strain tensor.
   */
  virtual void computeStrain(ColumnMajorMatrix & strain);
};

#endif //PLASTICMATERIAL_H
