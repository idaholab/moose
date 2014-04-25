/*
 * RavenTools.h
 *
 *  Created on: May 28, 2013
 *      Author: alfoa
 */

#ifndef RAVENTOOLS_H_
#define RAVENTOOLS_H_

#include "Interpolation_Functions.h"
#include "MooseObject.h"
#include "RavenTools_min.h"


template<>
InputParameters validParams<RavenTools>();

class RavenTools : public MooseObject
{
public:
   //> constructor for built-in raven tools
   RavenTools(const std::string & name, InputParameters parameters);

   virtual ~RavenTools();
   /*
    * All variables except status ones
    */
   double  getVariable(std::string variableName);                     ///< getVariable from mapping
   void updateVariable(std::string variableName, double & newValue);  ///< update variable into the mapping//   void updateVariable(const char variableName, double & newValue);
   virtual double compute(double value);
   std::string & getType();                                             ///< Get RavenTool type
   std::vector<std::string> getVariableNames();                         ///< Get variable Names

protected:
   std::string                       _type;               ///< RavenTools' type
   std::map <std::string,double>     _tool_parameters  ;  ///< RavenTools' parameters
   //void addVariable(std::string & variableName, double & newValue);
};

#endif /* RAVENTOOLS_H_ */
