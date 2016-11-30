/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWNODALVALUEMATERIAL_H
#define POROUSFLOWNODALVALUEMATERIAL_H

#include "Material.h"
#include "MaterialData.h"

//Forward Declarations
class PorousFlowNodalValueMaterial;

template<>
InputParameters validParams<PorousFlowNodalValueMaterial>();

class PorousFlowNodalValueMaterial : public Material
{
public:
  PorousFlowNodalValueMaterial(const InputParameters & parameters);

protected:
  virtual void initStatefulProperties(unsigned int n_points) override;
  virtual void computeProperties() override;

  /// resize the nodal properties every time computeProperties is called
  const bool _always_resize;

  /**
   * Counter variable for the nodes in an element: analogous
   * to _qp for quadpoint properties
   */
  unsigned _nodenum;

  /**
   * Derived classes may override this to specify which stateful properties
   * should be resized to the number of nodes in the current element.
   * This function is called during initStatefulProperties, and should
   * resize all the Stateful properties that are initialized in
   * initNodalStatefulProperties.
   */
  virtual void sizeNodalStatefulProperties();

  /**
   * Derived classes may override this to specify the initial
   * value of nodal stateful properties, in the analogous way
   * that initQpStatefulProperties initialises quad-point properties.
   * This function is called during initStatefulProperties, and the
   * properties that are initialized here should have been resized
   * using sizeNodalStatefulProperties
   */
  virtual void initNodalStatefulProperties();

  /**
   * Derived classes may override this to specify which properties
   * should be resized to the number of nodes in the current element.
   * This function is called during computeProperties, and should resize
   * all the properties that are computed in computeNodalProperties.
   * To resize a specific named property, derived classes could use
   * sizeNodalProperty.
   */
  virtual void sizeNodalProperties();

  /**
   * Derived classes may override this to compute nodal properties
   * in exactly the same way as computeQpProperties does for quadpoint
   * properties.  The nodal properties computed here must have been
   * resized using sizeNodalProperty
   */
  virtual void computeNodalProperties();

  /**
   * Makes property with name prop_name to be size equal to the
   * number of nodes in the current element
   */
  void sizeNodalProperty(const std::string & prop_name);

  /**
   * Makes all requested properties for this material to be size
   * equal to tht enumber of nodes in the current element
   */
  void sizeAllRequestedProperties();
};

#endif //POROUSFLOWNODALVALUEMATERIAL_H
