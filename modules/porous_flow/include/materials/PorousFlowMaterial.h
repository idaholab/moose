/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWMATERIAL_H
#define POROUSFLOWMATERIAL_H

#include "Material.h"
#include "MaterialData.h"
#include "PorousFlowDictator.h"

// Forward Declarations
class PorousFlowMaterial;

template <>
InputParameters validParams<PorousFlowMaterial>();

class PorousFlowMaterial : public Material
{
public:
  PorousFlowMaterial(const InputParameters & parameters);

protected:
  virtual void initStatefulProperties(unsigned int n_points) override;
  virtual void computeProperties() override;

  /// whether the derived class holds nodal values
  const bool _nodal_material;

  /// The variable names UserObject for the PorousFlow variables
  const PorousFlowDictator & _dictator;

  /**
   * Makes property with name prop_name to be size equal to the
   * number of nodes in the current element
   */
  void sizeNodalProperty(const std::string & prop_name);

  /**
   * Makes all supplied properties for this material to be size
   * equal to the number of nodes in the current element
   */
  void sizeAllSuppliedProperties();

  /**
   * Find the nearest quadpoint to the node labelled by nodenum
   * in the current element
   * @param nodenum the node number in the current element
   * @return the nearest quadpoint
   */
  unsigned nearestQP(unsigned nodenum) const;
};

#endif // POROUSFLOWMATERIAL_H
