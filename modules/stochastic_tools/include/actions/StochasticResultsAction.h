//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * This action is a crutch that gets around a construction and initialSetup execution order of
 * operations problem between three types of objects.
 *
 * SPT = SamplerPostprocessorTransfer (MultiappTransfer)
 * SR  = StochasticResults (VectorPostprocessor)
 * PCT = PolynomialChaosTrainer (GeneralUserObject)
 *
 * # PROBLEM
 *
 * Here is the scenario (see surrogats/load_store/train.i)
 *
 * 1. The following objects and data needed:
 *
 *    SPT: transfers PP values from sub-applications to the the master SR. The names of the
 *    vectors
 *         on the master are dictated by the PP names being captured from the sub-app.
 *    SR: stores vectors of stochastic data from the SPT
 *    PCT: uses the data from the SR to train a reduced order model
 *
 *    A problem arises because the SPT dictates the what vectors are declared in the SR.
 *
 * 2. Construction
 *
 * The following construction order exists as dictated by MOOSE: PCT, SPT, SR
 *
 * Therefore, the SPT must wait until initialSetup to declare the vectors on the SR because
 * the SR is not constructed until after the SPT.
 *
 * The PCT needs to query information about the data vectors in SR (e.g., size, is distributed),
 * which needs to happen in initialSetup because the SR is not constructed until after the PCT.
 *
 * 3. initialSetup
 *
 * The following initialSetup order exists as dictated by MOOSE: PCT, SR, SPT
 *
 * As mentioned above the SPT::initialSetup creates vectors in the SR. However, the PCT needs
 * the vectors in-place during initialSetup. Therefore, the current API doesn't
 * meet the needs for this workflow.
 *
 * # SOLUTION
 *
 * The first implementation of this relationship required that both the SPT and the SR
 * required the sampe Sampler object. The name of the Sampler was used as the name of the result
 * vectors in the SR. This worked, but limited the ability to retrieve more than one PP value from
 * a sub-application. Multiple values were only possible if the sub-application that was computing
 * them was based on another Sampler object. That scenario is far less likely then trying to
 * collect more the one PP value from a sub-application. Furthermore, the sampler name is
 * irrelevant to the data being captured from the sub-applications.
 *
 * The goal is to have this SPT be able to grab many PP values from a sub-application and
 * store them in vectors in SR using the PP name as the name of the vector. This
 * basic goal can be accomplished by using the initialSetup functions of the SPT and
 * SRT, until the results are needed by another object, such as a Trainer, as shown above.
 *
 * There are a few possible solutions. It is important to state that the PCT should always
 * operate on generic VPP data to allow for training data to come from any VPP objects. So,
 * solutions that require PCT to have knowledge of the SR are not considered.
 *
 * 1. Construct the SR before the SPT. This allows for the SPT to declare the vectors at
 *    construction, thus they are available during the intialSetup of other objects. The downside
 *    is that this will require a special Action for the SR construction.
 * 2. The PCT could use initialize() for querying the SR vectors. The downside is that the PCT
 *    is an example of a user-created surrogate trainer. Requiring the use of initialize() to
 *    get things working is not desirable for users code.
 * 3. Declare the vectors in the constructor of the SR. The downside is that the vector names
 *    are dictated by the SPT "from_postprocessor" input parameter, so there is no direct way to
 *    get them up front.
 * 4. Use an Action to set the a parameter on the SR based on a parameter for SPT.
 *
 * The solution implemented takes the approach of (4). When the SPT is created it caches the
 * PP names associated with the SR object that will be populated in this action. Then when the SR
 * is created it declares the vectors in the cache.
 */
class StochasticResultsAction : public Action
{
public:
  static InputParameters validParams();
  StochasticResultsAction(const InputParameters & params);
  virtual void act() override;
};
