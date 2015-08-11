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
  PFCTradMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  unsigned int _order;
  MaterialProperty<Real> & _M;
  MaterialProperty<Real> & _a;
  MaterialProperty<Real> & _b;
  MaterialProperty<Real> & _C0;
  MaterialProperty<Real> & _C2;
  MaterialProperty<Real> & _C4;
  MaterialProperty<Real> & _C6;
  MaterialProperty<Real> & _C8;
};

#endif //PFCTRADMATERIAL_H
