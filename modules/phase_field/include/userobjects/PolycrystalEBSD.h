/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALEBSD_H
#define POLYCRYSTALEBSD_H

#include "PolycrystalUserObjectBase.h"

// Forward Declarations
class PolycrystalEBSD;
class EBSDReader;

template <>
InputParameters validParams<PolycrystalEBSD>();

class PolycrystalEBSD : public PolycrystalUserObjectBase
{
public:
  PolycrystalEBSD(const InputParameters & parameters);

  virtual void getGrainsBasedOnPoint(const Point & point,
                                     std::vector<unsigned int> & grains) const override;
  virtual Real getVariableValue(unsigned int op_index, const Point & p) const override;
  virtual Real getNodalVariableValue(unsigned int op_index, const Node & n) const override;
  virtual unsigned int getNumGrains() const override;

protected:
  const unsigned int _phase;
  const EBSDReader & _ebsd_reader;
  const std::map<dof_id_type, std::vector<Real>> & _node_to_grain_weight_map;
};

#endif // POLYCRYSTALEBSD_H
