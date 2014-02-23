/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSPIECEWISELINEARSINK
#define RICHARDSPIECEWISELINEARSINK

#include "IntegratedBC.h"
#include "LinearInterpolation.h"
#include "RichardsPorepressureNames.h"
#include "Function.h"

// Forward Declarations
class RichardsPiecewiseLinearSink;

template<>
InputParameters validParams<RichardsPiecewiseLinearSink>();

class RichardsPiecewiseLinearSink : public IntegratedBC
{
public:

  RichardsPiecewiseLinearSink(const std::string & name,
                        InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  bool _use_mobility;
  bool _use_relperm;
  LinearInterpolation _sink_func;
  Function * const _m_func;

  const RichardsPorepressureNames & _pp_name_UO;
  unsigned int _pvar;

  MaterialProperty<std::vector<Real> > &_viscosity;
  MaterialProperty<RealTensorValue> & _permeability;
  MaterialProperty<std::vector<std::vector<Real> > > &_dseff;
  MaterialProperty<std::vector<Real> > &_rel_perm;
  MaterialProperty<std::vector<Real> > &_drel_perm;
  MaterialProperty<std::vector<Real> > &_density;
  MaterialProperty<std::vector<Real> > &_ddensity;


};

#endif //RichardsPiecewiseLinearSink
