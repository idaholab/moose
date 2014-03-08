#ifndef CLSHPLASTICMATERIAL_H
#define CLSHPLASTICMATERIAL_H

#include "SolidModel.h"

/**
 * Plastic material
 */
class CLSHPlasticMaterial : public SolidModel
{
public:
  CLSHPlasticMaterial(std::string name,
                      InputParameters parameters);

protected:

};

template<>
InputParameters validParams<CLSHPlasticMaterial>();

#endif //CLSHPLASTICMATERIAL_H
