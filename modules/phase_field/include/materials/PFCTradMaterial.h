#ifndef PFCTRADMATERIAL_H
#define PFCTRADMATERIAL_H

#include "Material.h"

//Forward Declarations
class PFCTradMaterial;

template<>
InputParameters validParams<PFCTradMaterial>();

class PFCTradMaterial : public Material
{
public:
  PFCTradMaterial(const std::string & name,
          InputParameters parameters);
  
protected:
  virtual void computeProperties();

private:  
  MaterialProperty<Real> & _a;
  MaterialProperty<Real> & _b;
  MaterialProperty<Real> & _C0;
  MaterialProperty<Real> & _C2;
  MaterialProperty<Real> & _C4;
  
};

#endif //PFCTRADMATERIAL_H
