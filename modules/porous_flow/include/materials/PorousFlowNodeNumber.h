/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWNODENUMBER_H
#define POROUSFLOWNODENUMBER_H

#include "Material.h"

//Forward Declarations
class PorousFlowNodeNumber;

template<>
InputParameters validParams<PorousFlowNodeNumber>();

/**
 * PorousFlow Materials store nodal information (such as porepressures,
 * densities, etc) at quadpoints.  This Material contains the map
 * that defines which quadpoint stores which node's information.
 * _node_number[qp] = the element's node number that is stored at quadpoint=qp
 *
 * This Material also checks that the number of nodes does not exceed
 * the number of quadpoints in each element, for elements not on
 * a boundary.  If the check fails then clearly the other PorousFlow
 * Materials won't work (there won't be enough quadpoints to store all
 * the required nodal information).
 *
 * Finally, the current implementation is that for all elements not on
 * a boundary (_bnd=false) _node_number[qp] = qp.  This is so that
 * "volumetric" MOOSE objects such as Kernels can also make that
 * simple assumption without having to painfully figure out which
 * quadpoint holds information about which node (viz, they'd have
 * to construct the inverse map to _node_number).
*/
class PorousFlowNodeNumber : public Material
{
public:
  PorousFlowNodeNumber(const InputParameters & parameters);

protected:
  /// calculate the node number only upon initial (this is only OK to use for simulations that aren't using mesh adaptivity)
  const bool _on_initial_only;

  /// node number for each quadpoint
  MaterialProperty<unsigned int> & _node_number;

  /// node number for each quadpoint
  MaterialProperty<unsigned int> & _node_number_old;

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  /**
   * Calculate the node number to a given quadpoint
   * @return the node number
   */
  virtual unsigned nearest();

};

#endif //POROUSFLOWNODENUMBER_H
