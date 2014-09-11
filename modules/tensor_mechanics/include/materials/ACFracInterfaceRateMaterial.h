#ifndef ACFRACINTERFACERATEMATERIAL_H
#define ACFRACINTERFACERATEMATERIAL_H

#include "Material.h"

class ACFracInterfaceRateMaterial;

template<>
InputParameters validParams<ACFracInterfaceRateMaterial>();

class ACFracInterfaceRateMaterial : public Material
{
 public:
  ACFracInterfaceRateMaterial(const std::string & name,
                             InputParameters parameters);

 protected:
  virtual void computeQpProperties();

  MaterialProperty<Real> & _L;
  MaterialProperty<Real> & _kappa_op;

  Real _L0;
  Real _l;

 private:

};

#endif //ACFRACINTERFACERATEMATERIAL_H
