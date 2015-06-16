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

#ifndef ERRORLOGGER_H
#define ERRORLOGGER_H

#include <vector>
#include <string>

using namespace std;

/**
 * An ErrorVector is a vector template typed for string objects.
 */
//typedef vector<string> ErrorVector;

/**
 * The ErrorLogger class has functions to store errors in a Vector&lt;string&gt;
 * attribute and then dump those errors to a file.
 */
class ErrorLogger {

private:

    /**
     * A ErrorVector object to contain string errors.
     */
	vector<string> errorVector;

public:

    /**
     * The Constructor.
     */
    ErrorLogger();

    /**
     * The Destructor.
     */
    ~ErrorLogger();

    /**
     * Appends an error string to the errorVector attribute.
     *
     * @param error An error string.
     */
    void logError(string error);

    /**
     * Dumps the errors in errorVector into a file with a filename format
     * "updatererrors_&lt;timestamp&gt;.log".
     *
     * @return The name of the error log file.
     */
    string dumpErrors();

};

#endif
