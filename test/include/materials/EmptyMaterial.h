#ifndef EMPTYMATERIAL_H_
#define EMPTYMATERIAL_H_

#include "Material.h"


/**
 * Empty material for use in simple applications that don't need material properties.
 */
class EmptyMaterial : public Material
{
public:
  EmptyMaterial(const std::string & name,
                InputParameters parameters);
  
protected:
  virtual void computeProperties();
};

template<>
InputParameters validParams<EmptyMaterial>();

#endif //EMPTYMATERIAL_H
