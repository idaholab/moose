#include "ADInterfaceKernel.h"

class FreundlichPenaltyInterface : public ADInterfaceKernel
{
public:
  static InputParameters validParams();
  FreundlichPenaltyInterface(const InputParameters &);

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;
  const ADMaterialProperty<Real> & _A;
  const ADMaterialProperty<Real> & _B;
  const ADMaterialProperty<Real> & _D;
  const ADMaterialProperty<Real> & _E;
  const ADMaterialProperty<Real> & _d1;
  const ADMaterialProperty<Real> & _d2;
  const ADMaterialProperty<Real> & _diff;
  const ADMaterialProperty<Real> & _diff_neighbor;
  const ADMaterialProperty<Real> & _carbon_density;
  const Real & _penalty;
  unsigned int _T_var;
  const VariableValue & _temperature;
};
