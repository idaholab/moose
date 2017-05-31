#ifndef SOLIDMATERIALPROPERTIES_H
#define SOLIDMATERIALPROPERTIES_H

#include "GeneralUserObject.h"
#include "Function.h"
#include "ZeroInterface.h"

class SolidMaterialProperties;

template <>
InputParameters validParams<SolidMaterialProperties>();

/**
 *
 */
class SolidMaterialProperties : public GeneralUserObject, public ZeroInterface
{
public:
  SolidMaterialProperties(const InputParameters & parameters);

  virtual void initialize();
  virtual void finalize();
  virtual void execute();

  Real k(Real temp) const;
  Real Cp(Real temp) const;
  Real rho(Real temp) const;

  Function & getKFunction() const { return _k; }
  Function & getCpFunction() const { return _Cp; }
  Function & getRhoFunction() const { return _rho; }

protected:
  Function & _k;
  Function & _Cp;
  Function & _rho;
};

#endif /* SOLIDMATERIALPROPERTIES_H */
