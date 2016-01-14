/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef FAILUREINDEXPD_H
#define FAILUREINDEXPD_H

#include "ElementUserObject.h"

class FailureIndexPD;

template<>
InputParameters validParams<FailureIndexPD>();

class FailureIndexPD : public ElementUserObject
{
public:
  FailureIndexPD(const InputParameters & parameters);

  ~FailureIndexPD(); // the destructor closes the output file

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & u );
  virtual void finalize();
  virtual Real ComputeFailureIndex(unsigned int nodeid) const;

protected:

  NumericVector<Number> & _IntactBonds;
  NumericVector<Number> & _TotalBonds;
  const MaterialProperty<Real> & _bond_status_old;

};

#endif // FAILUREINDEXPD_H
