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

#ifndef RENAMEBLOCK_H
#define RENAMEBLOCK_H

// MOOSE includes
#include "MeshModifier.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

// Forward declerations
class RenameBlock;

template<>
InputParameters validParams<RenameBlock>();

/**
 * MeshModifier for re-numbering or re-naming blocks
 */
class RenameBlock : public MeshModifier
{
public:

  /**
   * Class constructor
   * @param name The name of the RenameBlock
   * @param parameters The input parameters
   */
  RenameBlock(const std::string & name, InputParameters parameters);

  /**
   * Class destructor
   */
  virtual ~RenameBlock();

  /// Perform the actual subdomain modification
  virtual void modify();

private:

  std::vector<SubdomainID> _old_block_id;

  std::vector<SubdomainID> _new_block_id;

  std::vector<SubdomainName> _new_block_name;
};

#endif // RENAMEBLOCK_H
