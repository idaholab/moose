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

#ifndef LIBCURLUTILS_H
#define LIBCURLUTILS_H

#include <iostream>
#include <string>
#include <curl/curl.h>

using namespace std;

/**
 * LibcurlUtils is a utility class used to transmit and receive information
 * with the POST and GET HTTP methods using the C library libcurl.
 */
class LibcurlUtils {

private:

    /**
     * A handle to the cURL object.
     */
    CURL * curl;

    /**
     * The result of the last curl call.
     */
    CURLcode result;

    /**
     * A char array an error from the last curl call.
     */
    char error[CURL_ERROR_SIZE];

    /**
     * A flag indicating whether or not cURL calls will skip peer
     * certificate verification for HTTPS urls. This flag should
     * only be set to true for testing purposes.
     */
    bool ignoreSslPeerVerification;

public:

    /**
     * The Constructor.
     */
    LibcurlUtils();

    /**
     * The Destructor.
     */
    ~LibcurlUtils();

    /**
     * Uses libcurl and GET to return the contents located at url.
     *
     * @param url The URL of the GET request.
     * @param username The username. It is ignored if it is empty. It may not be null.
     * @param password The password. It is ignored if it is empty. It may not be null.
     * @return The contents at the URL or an error message if one took place.
     */
    string get(string url, string username, string password);

    /**
     * Uses libcurl and POST to transmit value at url.
     *
     * @param url The url that is used to post the value.
     * @param username The username. It is ignored if it is empty. It may not be null.
     * @param password The password. It is ignored if it is empty. It may not be null.
     * @param value The value that is posted to the url.
     * @return A string containing the error if one took place. Else returns an empty string.
     */
    string post(string url, string value, string username, string password);

    /**
     * Sets the ignoreSslPeerVerification flag. If ignoreSslPeerVerification flag is
     * set to true then cURL will skip peer certificate verification for HTTPS urls.
     * This flag should only be set to true for testing purposes.
     *
     * @param ignoreSslPeerVerification The value for the ignoreSslPeerVerification flag.
     */
    void setIgnoreSslPeerVerification(bool ignoreSslPeerVerification);

    /**
     * A callback required by the C libcurl library to write the contents returned by get() to a buffer.
     *
     * @param data The get data.
     * @param size The size of each item.
     * @param nmemb The number of items in memory.
     * @param buffer The buffer to store the get() contents.
     * @return The amount written which should be size * nmemb.
     */
    static int writeGetData(char * data, size_t size, size_t nmemb, string buffer);

};

#endif
