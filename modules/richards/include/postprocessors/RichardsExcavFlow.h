/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSEXCAVFLOW_H
#define RICHARDSEXCAVFLOW_H

#include "SideIntegralVariablePostprocessor.h"
#include "MaterialPropertyInterface.h"
#include "FunctionInterface.h"
#include "RichardsVarNames.h"

//Forward Declarations
class RichardsExcavFlow;
class Function;

template<>
InputParameters validParams<RichardsExcavFlow>();

/**
 * Records total mass flow into an excavation defined by a RichardsExcavGeom function
 */
class RichardsExcavFlow :
public SideIntegralVariablePostprocessor,
public FunctionInterface
{
public:
  RichardsExcavFlow(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();

  /// holds info regarding the Richards variables
  const RichardsVarNames & _richards_name_UO;

  /// the richards variable number for which we want the mass flow
  unsigned int _pvar;

  /// mass-flux of fluid (a vector in the multicomponent case)
  MaterialProperty<std::vector<RealVectorValue> > &_flux;

  /// the RichardsExcavGeom that defines where on the boundary we'll compute the mass flux
  Function & _func;
};

#endif // RICHARDSEXCAVFLOW_H
