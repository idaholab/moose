#pragma once

#include "Parser.h"
#include "InputParameters.h"

#include "hit.h"

#include <string>
#include <vector>
#include <string>
#include <iomanip>
#include <optional>

class InputParameters;
class GlobalParamsAction;
class CommandLine;

namespace Moose
{

/**
 * Base object for building objects.
 *
 * Contains the capability for turning hit input and command-line arguments
 * into objects through extraction.
 */
class BuilderBase
{
public:
  BuilderBase(std::shared_ptr<Parser> Parser);

  /**
   * @return The underlying Parser
   */
  ///@{
  const Parser & parser() const;
  Parser & parser();
  ///@}

  /**
   * Return the primary (first) filename that was parsed
   */
  std::string getPrimaryFileName(bool stripLeadingPath = true) const;

  /**
   * @return The root node from the parser
   */
  hit::Node & root() { return parser().root(); }

  /**
   * Parses the hit command-line arguments from \p command_line into a hit tree.
   *
   * @return The root node of rhe tree
   */
  static std::unique_ptr<hit::Node> parseCLIArgs(CommandLine & command_line);

protected:
  /**
   * Extracts the parameters for an object with hit prefix \p prefix into the parameters
   * \p p.
   *
   * To account for global parameters, provide \p global_params_block and
   * \p global_params_block_name.
   */
  void extractParams(const std::string & prefix,
                     InputParameters & p,
                     GlobalParamsAction * const global_params_block,
                     const std::optional<std::string> global_params_block_name);

  /**
   * Merges the hit command line arguments from \p command_line into the root tree
   * stored at root().
   *
   * @return The root node of the tree that represents just the filtered hit params
   */
  std::unique_ptr<hit::Node> mergeCLIArgs(CommandLine & command_line);

  /// The parser we're getting input from
  const std::shared_ptr<Parser> _parser;

  /// The set of all variables extracted from the input file
  std::set<std::string> _extracted_vars;

  /// Collection point for errors
  std::string _errmsg;
  /// Collection point for warnings
  std::string _warnmsg;

private:
  /**************************************************************************************************
   **************************************************************************************************
   *                                   Parameter Extraction Routines *
   **************************************************************************************************
   **************************************************************************************************/

  /**
   * Helper functions for setting parameters of arbitrary types - bodies are in the .C file
   * since they are called only from this Object
   */
  /// Template method for setting any scalar type parameter read from the input file or command line
  template <typename T, typename Base>
  void setScalarParameter(const std::string & full_name,
                          const std::string & short_name,
                          InputParameters::Parameter<T> * param,
                          bool in_global,
                          GlobalParamsAction * global_block);

  template <typename T, typename UP_T, typename Base>
  void setScalarValueTypeParameter(const std::string & full_name,
                                   const std::string & short_name,
                                   InputParameters::Parameter<T> * param,
                                   bool in_global,
                                   GlobalParamsAction * global_block);

  /// Template method for setting any vector type parameter read from the input file or command line
  template <typename T, typename Base>
  void setVectorParameter(const std::string & full_name,
                          const std::string & short_name,
                          InputParameters::Parameter<std::vector<T>> * param,
                          bool in_global,
                          GlobalParamsAction * global_block);

  /// Template method for setting any map type parameter read from the input file or command line
  template <typename KeyType, typename MappedType>
  void setMapParameter(const std::string & full_name,
                       const std::string & short_name,
                       InputParameters::Parameter<std::map<KeyType, MappedType>> * param,
                       bool in_global,
                       GlobalParamsAction * global_block);

  /**
   * Sets an input parameter representing a file path using input file data.  The file path is
   * modified to be relative to the directory this application's input file is in.
   */
  template <typename T>
  void setFilePathParam(const std::string & full_name,
                        const std::string & short_name,
                        InputParameters::Parameter<T> * param,
                        InputParameters & params,
                        bool in_global,
                        GlobalParamsAction * global_block);

  /**
   * Sets an input parameter representing a vector of file paths using input file data.  The file
   * paths are modified to be relative to the directory this application's input file is in.
   */
  template <typename T>
  void setVectorFilePathParam(const std::string & full_name,
                              const std::string & short_name,
                              InputParameters::Parameter<std::vector<T>> * param,
                              InputParameters & params,
                              bool in_global,
                              GlobalParamsAction * global_block);
  /**
   * Template method for setting any double indexed type parameter read from the input file or
   * command line.
   */
  template <typename T>
  void setDoubleIndexParameter(const std::string & full_name,
                               const std::string & short_name,
                               InputParameters::Parameter<std::vector<std::vector<T>>> * param,
                               bool in_global,
                               GlobalParamsAction * global_block);

  /**
   * Template method for setting any triple indexed type parameter read from the input file or
   * command line.
   */
  template <typename T>
  void setTripleIndexParameter(
      const std::string & full_name,
      const std::string & short_name,
      InputParameters::Parameter<std::vector<std::vector<std::vector<T>>>> * param,
      bool in_global,
      GlobalParamsAction * global_block);

  /**
   * Template method for setting any multivalue "scalar" type parameter read from the input file or
   * command line.  Examples include "Point" and "RealVectorValue".
   */
  template <typename T>
  void setScalarComponentParameter(const std::string & full_name,
                                   const std::string & short_name,
                                   InputParameters::Parameter<T> * param,
                                   bool in_global,
                                   GlobalParamsAction * global_block);

  /**
   * Template method for setting several multivalue "scalar" type parameter read from the input
   * file or command line.  Examples include "Point" and "RealVectorValue".
   */
  template <typename T>
  void setVectorComponentParameter(const std::string & full_name,
                                   const std::string & short_name,
                                   InputParameters::Parameter<std::vector<T>> * param,
                                   bool in_global,
                                   GlobalParamsAction * global_block);

  /**
   * Template method for setting vector of several multivalue "scalar" type parameter read from the
   * input file or command line.  Examples include vectors of several "Point"s and
   * "RealVectorValue"s such as (a three-element vector; each element is several "Point"s):
   * points_values = '0 0 0
   *                  0 0 1;
   *                  0 1 0;
   *                  1 0 0
   *                  1 1 0
   *                  1 1 1'
   */
  template <typename T>
  void
  setVectorVectorComponentParameter(const std::string & full_name,
                                    const std::string & short_name,
                                    InputParameters::Parameter<std::vector<std::vector<T>>> * param,
                                    bool in_global,
                                    GlobalParamsAction * global_block);

  /// The current parameter object for which parameters are being extracted
  InputParameters * _current_params;

  /// The current stream object used for capturing errors during extraction
  std::ostringstream * _current_error_stream;

  /// Tracks whether a deprecated param has had its warning message printed already
  std::unordered_set<std::string> _deprec_param_tracker;
};

}
