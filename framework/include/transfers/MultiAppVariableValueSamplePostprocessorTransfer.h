//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppTransfer.h"
#include "MeshChangedInterface.h"

/**
 * Samples a variable's value in the parent application domain at the point where
 * the MultiApp (for each child app) is. Copies that value into a postprocessor in the MultiApp.
 */
class MultiAppVariableValueSamplePostprocessorTransfer : public MultiAppTransfer,
                                                         public MeshChangedInterface
{
public:
  static InputParameters validParams();

  MultiAppVariableValueSamplePostprocessorTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void execute() override;

  void meshChanged() override;

protected:
  /**
   * Sets up the postprocessor to processor ID communication pattern data member \p
   * _postprocessor_to_processor_id
   */
  void setupPostprocessorCommunication();

  /**
   * Method that caches data regarding the element to postprocess relationship. This is an expensive
   * method, so we only do it during initial setup or if the mesh changes
   */
  void cacheElemToPostprocessorData();

  /// the name of the postprocessor on the sub-applications
  PostprocessorName _postprocessor_name;
  /// the name of the variable on the main-application
  AuxVariableName _var_name;
  /// the component number of the variable for transferring
  unsigned int _comp;
  /// the moose variable
  MooseVariableFieldBase & _var;
  // sub-application ids of all local active element of the main-application
  std::vector<unsigned int> _cached_multiapp_pos_ids;

  /// Entries in this vector correspond to the processor ID that owns the application/postprocessor
  /// corresponding to the index
  std::vector<processor_id_type> _postprocessor_to_processor_id;

  /// The postprocessors that this process needs for its active local elements
  std::unordered_set<unsigned int> _needed_postprocessors;
};
