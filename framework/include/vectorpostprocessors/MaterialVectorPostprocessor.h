/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MATERIALVECTORPOSTPROCESSOR_H
#define MATERIALVECTORPOSTPROCESSOR_H

#include "ElementVectorPostprocessor.h"

//Forward Declarations
class MaterialVectorPostprocessor;

template<>
InputParameters validParams<MaterialVectorPostprocessor>();

class MaterialVectorPostprocessor :
  public ElementVectorPostprocessor
{
public:
  MaterialVectorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override { };
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

private:
  void sortVecs();

  std::set<unsigned int> _elem_filter;
  VectorPostprocessorValue & _elem_ids;
  VectorPostprocessorValue & _qp_ids;
  std::vector<VectorPostprocessorValue *> _prop_vecs;
  std::vector<const MaterialProperty<Real>*> _prop_refs;
};

#endif
