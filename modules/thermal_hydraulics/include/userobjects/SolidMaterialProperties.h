#pragma once

#include "GeneralUserObject.h"
#include "Function.h"

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
  Real cp(Real temp) const;
  Real rho(Real temp) const;

  const Function & getKFunction() const { return _k; }
  const Function & getCpFunction() const { return _cp; }
  const Function & getRhoFunction() const { return _rho; }

protected:
  const Function & _k;
  const Function & _cp;
  const Function & _rho;

public:
  static InputParameters validParams();
};
