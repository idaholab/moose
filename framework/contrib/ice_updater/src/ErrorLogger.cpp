/*******************************************************************************
 * Copyright (c) 2015, UT-Battelle, LLC.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in
 *  the documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *  contributors may be used to endorse or promote products derived
 *  from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*******************************************************************************/
#include "ErrorLogger.h"
#include <time.h>
#include <fstream>
#include <iostream>


/**
 * Appends an error string to the errorVector attribute.
 *
 * @param error An error string.
 */
void ErrorLogger::logError(std::string error)
{
  // The following block is modified from the C++ reference for the strftime function
  // and is used to generate a formatted timestamp (e.g., Thu Aug 23 14:55:02 2001)
  // to prepend to the error.
  time_t rawtime;
  struct tm * timeinfo;
  char buffer [80];
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer, 80, "%c",timeinfo);

  // Get the timestamp from the buffer.
  std::string timestamp = std::string(buffer);

  // Create an error string that is padded on the left by [timestamp].
  std::string formattedError = "[";
  formattedError += timestamp;
  formattedError += "] ";
  formattedError += error;

  // Add the timestamp formatted error to the errorVector.
  errorVector.push_back(formattedError);
}

/**
 * Dumps the errors in errorVector into a file with a filename format
 * "updatererrors_<timestamp>.log".
 *
 * @return The name of the error log file or a blank string if no errors have been recorded.
 */
std::string ErrorLogger::dumpErrors()
{
  // If there are no errors in the errorVector,
  // just return a blank string.
  if (errorVector.empty())
    return "";

  // Declare a variable to store the contents of the error log file.
  std::string errorFileContents = "";

  // Loop over the errors in errorVector.
  for (unsigned int i=0; i<errorVector.size(); i++)
  {
    // Append the error and a new line char to the error file contents.
    errorFileContents += errorVector[i];
    errorFileContents += "\n";
  }

  // Clear out all of the errors.
  errorVector.clear();

  // The following block is modified from the C++ reference for the strftime function
  // and is used to generate a formatted timestamp (20120428_130456)
  // to use in the error log filename.
  time_t rawtime;
  struct tm * timeinfo;
  char buffer [80];
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer, 80, "%Y%m%d_%H%M%S",timeinfo);

  // Get the timestamp from the buffer.
  std::string timestamp = std::string(buffer);

  // Create a filename from the timestamp for writing the errors.
  std::string errorFileName = "updatererrors_";
  errorFileName += timestamp;
  errorFileName += ".log";

  // Write errors to file.
  std::ofstream errorFile;

  // Open the file.
  errorFile.open(errorFileName.data());

  // Write the contents to the error file.
  errorFile << errorFileContents;

  // Close the file.
  errorFile.close();

  // Return the name of the error log file.
  return errorFileName;
}
