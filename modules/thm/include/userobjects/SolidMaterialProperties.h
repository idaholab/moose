#ifndef SOLIDMATERIALPROPERTIES_H
#define SOLIDMATERIALPROPERTIES_H

#include "GeneralUserObject.h"
#include "Function.h"
#include "ZeroInterface.h"

class SolidMaterialProperties;

template<>
InputParameters validParams<SolidMaterialProperties>();

/**
 *
 */
class SolidMaterialProperties :
  public GeneralUserObject,
  public ZeroInterface
{
public:
  SolidMaterialProperties(const InputParameters & parameters);
  virtual ~SolidMaterialProperties();

  virtual void initialize();
  virtual void finalize();
  virtual void execute();

  Real k(Real temp) const;
  Real Cp(Real temp) const;
  Real rho(Real temp) const;

protected:
  const Real & _k_const;
  const Real & _Cp_const;
  const Real & _rho_const;

  Function * _k;
  Function * _Cp;
  Function * _rho;

  /**
   * Helper function so we can avoid calling getParam on parameters
   * before they are valid.
   */
  const Real & setConstRefParam(std::string get_string, std::string set_string);
};



#endif /* SOLIDMATERIALPROPERTIES_H */
