#ifndef SOLIDMATERIALPROPERTIES_H
#define SOLIDMATERIALPROPERTIES_H

#include "GeneralUserObject.h"
#include "Function.h"

class SolidMaterialProperties;

template<>
InputParameters validParams<SolidMaterialProperties>();

/**
 *
 */
class SolidMaterialProperties : public GeneralUserObject
{
public:
  SolidMaterialProperties(const std::string & name, InputParameters parameters);
  virtual ~SolidMaterialProperties();

  virtual void initialize();
  virtual void finalize();
  virtual void destroy();
  virtual void execute();

  Real k(Real temp) const;
  Real Cp(Real temp) const;
  Real rho(Real temp) const;

protected:
  Function * _k;
  const Real & _k_const;
  Function * _Cp;
  const Real & _Cp_const;
  Function * _rho;
  const Real & _rho_const;
};



#endif /* SOLIDMATERIALPROPERTIES_H */
