#ifndef AXISYMMETRICRZ_H
#define AXISYMMETRICRZ_H

#include "Element.h"

//Forward Declarations
class SymmElasticityTensor;
namespace SolidMechanics
{

class AxisymmetricRZ : public Element
{
public:
  AxisymmetricRZ(SolidModel & solid_model, const std::string & name, InputParameters parameters);
  virtual ~AxisymmetricRZ();

protected:

  virtual void computeStrain( const unsigned qp,
                              const SymmTensor & total_strain_old,
                              SymmTensor & total_strain_new,
                              SymmTensor & strain_increment );

  virtual unsigned int getNumKnownCrackDirs() const
  {
    return 1;
  }

  VariableValue & _disp_r;
  VariableValue & _disp_z;

  const bool _large_strain;

  VariableGradient & _grad_disp_r;
  VariableGradient & _grad_disp_z;

};

}

#endif //SOLIDMECHANICSMATERIALRZ_H
