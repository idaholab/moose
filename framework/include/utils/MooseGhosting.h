//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

class InputParameters;

namespace Moose
{
enum class RelationshipManagerType : unsigned char;

/**
 * This returns an \p InputParameters object containing an \p ElementSideNeighborLayers relationship
 * manager with zero layers of ghosting. While zero layers may seem foolish, this is actually very
 * useful for building the correct sparsity pattern between intra-element degrees of freedom
 * @param rm_type The type of relationship manager that should be added. This can be GEOMETRIC,
 *                ALGEBRAIC, COUPLING or a combination of the three
 */
InputParameters zeroLayerGhosting(RelationshipManagerType rm_type);

/**
 * This returns an \p InputParameters object containing an \p ElementSideNeighborLayers relationship
 * manager with one side layer of ghosting.
 * @param rm_type The type of relationship manager that should be added. This can be GEOMETRIC,
 *                ALGEBRAIC, COUPLING or a combination of the three
 */
InputParameters oneLayerGhosting(RelationshipManagerType rm_type);
}
