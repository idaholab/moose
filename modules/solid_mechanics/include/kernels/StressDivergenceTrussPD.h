/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef STRESSDIVERGENCETRUSSPD_H
#define STRESSDIVERGENCETRUSSPD_H

#include "Kernel.h"

//Forward Declarations
class ColumnMajorMatrix;
class SymmElasticityTensor;
class SymmTensor;

class StressDivergenceTrussPD : public Kernel
{
public:

  StressDivergenceTrussPD(const std::string & name, InputParameters parameters);
  virtual ~StressDivergenceTrussPD() {}

protected:
  virtual void initialSetup();
  virtual void computeResidual();
  virtual Real computeQpResidual() {return 0;}

  virtual void computeJacobian();

  virtual void computeOffDiagJacobian(unsigned int jvar);

  void computeStiffness(ColumnMajorMatrix & stiff_global);

  const MaterialProperty<Real> & _axial_force;
  const MaterialProperty<Real> & _stiff_elem;
  const MaterialProperty<Real> & _bond_status;

private:
  const unsigned int _component;

  const bool _xdisp_coupled;
  const bool _ydisp_coupled;
  const bool _zdisp_coupled;
  const bool _temp_coupled;

  const unsigned int _xdisp_var;
  const unsigned int _ydisp_var;
  const unsigned int _zdisp_var;
  const unsigned int _temp_var;

  const std::vector<RealGradient> * _orientation;

};

template<>
InputParameters validParams<StressDivergenceTrussPD>();

#endif //STRESSDIVERGENCETRUSSPD_H
