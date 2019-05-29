#pragma once

#include "GeneralUserObject.h"
#include "Function.h"

class SolidMaterialProperties;

template <>
InputParameters validParams<SolidMaterialProperties>();

/**
 *
 */
class SolidMaterialProperties : public GeneralUserObject
{
public:
  SolidMaterialProperties(const InputParameters & parameters);

  virtual void initialize();
  virtual void finalize();
  virtual void execute();

  Real k(Real temp) const;
  Real Cp(Real temp) const;
  Real rho(Real temp) const;

  const Function & getKFunction() const { return _k; }
  const Function & getCpFunction() const { return _Cp; }
  const Function & getRhoFunction() const { return _rho; }

protected:
  const Function & _k;
  const Function & _Cp;
  const Function & _rho;
};
