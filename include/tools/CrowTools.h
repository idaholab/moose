/*
 * CrowTools.h
 *
 *  Created on: May 28, 2013
 *      Author: alfoa
 */

#ifndef CROWTOOLS_H_
#define CROWTOOLS_H_

#include "Interpolation_Functions.h"
#include "MooseObject.h"
#include "CrowTools_min.h"


template<>
InputParameters validParams<CrowTools>();

class CrowTools : public MooseObject
{
public:
   //> constructor for built-in crow tools
   CrowTools(const std::string & name, InputParameters parameters);

   virtual ~CrowTools();
   /*
    * All variables except status ones
    */
   double  getVariable(std::string variableName);                     ///< getVariable from mapping
   void updateVariable(std::string variableName, double & newValue);  ///< update variable into the mapping//   void updateVariable(const char variableName, double & newValue);
   virtual double compute(double value);
   std::string & getType();                                             ///< Get CrowTool type
   std::vector<std::string> getVariableNames();                         ///< Get variable Names

protected:
   std::string                       _type;               ///< CrowTools' type
   std::map <std::string,double>     _tool_parameters  ;  ///< CrowTools' parameters
   //void addVariable(std::string & variableName, double & newValue);
};

#endif /* CROWTOOLS_H_ */
