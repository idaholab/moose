#ifndef PFCRFFMATERIAL_H
#define PFCRFFMATERIAL_H

#include "Material.h"

//Forward Declarations
class PFCRFFMaterial;

template<>
InputParameters validParams<PFCRFFMaterial>();

class PFCRFFMaterial : public Material
{
public:
  PFCRFFMaterial(const std::string & name,
          InputParameters parameters);
  
protected:
  virtual void computeQpProperties();

private:
  MaterialProperty<Real> & _M;
  MaterialProperty<Real> & _alpha_R_0;
  MaterialProperty<Real> & _alpha_I_0;
  MaterialProperty<Real> & _A_R_0;
  MaterialProperty<Real> & _A_I_0;
  MaterialProperty<Real> & _alpha_R_1;
  MaterialProperty<Real> & _alpha_I_1;
  MaterialProperty<Real> & _A_R_1;
  MaterialProperty<Real> & _A_I_1;
  MaterialProperty<Real> & _alpha_R_2;
  MaterialProperty<Real> & _alpha_I_2;
  MaterialProperty<Real> & _A_R_2;
  MaterialProperty<Real> & _A_I_2;;
  
};

#endif //PFCRFFMATERIAL_H
