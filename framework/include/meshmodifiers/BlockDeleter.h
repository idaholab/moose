/****************************************************************/
/* DO NOT MODIFY THIS HEADER */
/* MOOSE - Multiphysics Object Oriented Simulation Environment */
/* */
/* (c) 2010 Battelle Energy Alliance, LLC */
/* ALL RIGHTS RESERVED */
/* */
/* Prepared by Battelle Energy Alliance, LLC */
/* Under Contract No. DE-AC07-05ID14517 */
/* With the U. S. Department of Energy */
/* */
/* See COPYRIGHT for full restrictions */
/****************************************************************/

#ifndef BLOCKDELETER_H
#define BLOCKDELETER_H

#include "MeshModifier.h"

class BlockDeleter;

template<>
InputParameters validParams<BlockDeleter>();

///Defines the block to be removed and then removes it
class BlockDeleter : public MeshModifier
{
  public:
  BlockDeleter(const InputParameters & parameters);
  ///Perform the actual subdomain modification
  void modify();
  private:
  SubdomainID _block_id;
};

#endif /* BLOCKDELETER_H */

