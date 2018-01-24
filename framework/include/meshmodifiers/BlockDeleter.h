//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BLOCKDELETER_H
#define BLOCKDELETER_H

#include "ElementDeleterBase.h"

class BlockDeleter;

template <>
InputParameters validParams<BlockDeleter>();

class BlockDeleter : public ElementDeleterBase
{
public:
  BlockDeleter(const InputParameters & parameters);

protected:
  virtual bool shouldDelete(const Elem * elem) override;

private:
  ///Defines the block to be removed
  const SubdomainID _block_id;
};

#endif /* BLOCKDELETER_H */
