/*******************************************************************************
 * Copyright (c) 2012, 2014 UT-Battelle, LLC.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *   Initial API and implementation and/or initial documentation - Jay Jay Billings,
 *   Jordan H. Deyton, Dasha Gorin, Alexander J. McCaskey, Taylor Patterson,
 *   Claire Saunders, Matthew Wang, Anna Wojtowicz
 *******************************************************************************/

#include "ErrorLogger.h"
#include <time.h>
#include <fstream>
#include <iostream>

/**
 * The Constructor.
 */
ErrorLogger::ErrorLogger() {

}

/**
 * The Destructor.
 */
ErrorLogger::~ErrorLogger() {
    return;
}

/**
 * Appends an error string to the errorVector attribute.
 *
 * @param error An error string.
 */
void ErrorLogger::logError(string error) {

    //The following block is modified from the C++ reference for the strftime function
    //and is used to generate a formatted timestamp (e.g., Thu Aug 23 14:55:02 2001)
    //to prepend to the error.
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%c",timeinfo);

    //Get the timestamp from the buffer.
    string timestamp = string(buffer);

    //Create an error string that is padded on the left by [timestamp].
    string formattedError = "[";
    formattedError += timestamp;
    formattedError += "] ";
    formattedError += error;

    //Add the timestamp formatted error to the errorVector.
    errorVector.push_back(formattedError);

    return;
}

/**
 * Dumps the errors in errorVector into a file with a filename format
 * "updatererrors_&lt;timestamp&gt;.log".
 *
 * @return The name of the error log file or a blank string if no errors have been recorded.
 */
string ErrorLogger::dumpErrors() {

    //If there are no errors in the errorVector.
    if(errorVector.empty()) {

        //Just return a blank string.
        return "";
    }

    //Declare a variable to store the contents of the error log file.
    string errorFileContents = "";

    //Loop over the errors in errorVector.
    for(int i=0; i<errorVector.size(); i++) {

        //Append the error and a new line char to the error file contents.
        errorFileContents += errorVector[i];
        errorFileContents += "\n";
    }

    //Clear out all of the errors.
    errorVector.clear();

    //The following block is modified from the C++ reference for the strftime function
    //and is used to generate a formatted timestamp (20120428_130456)
    //to use in the error log filename.
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%Y%m%d_%H%M%S",timeinfo);

    //Get the timestamp from the buffer.
    string timestamp = string(buffer);

    //Create a filename from the timestamp for writing the errors.
    string errorFileName = "updatererrors_";
    errorFileName += timestamp;
    errorFileName += ".log";

    //Write errors to file.
    ofstream errorFile;

    //Open the file.
    errorFile.open(errorFileName.data());

    //Write the contents to the error file.
    errorFile << errorFileContents;

    //Close the file.
    errorFile.close();

    //Return the name of the error log file.
    return errorFileName;
}
