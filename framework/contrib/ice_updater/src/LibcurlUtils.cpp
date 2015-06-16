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

#include "LibcurlUtils.h"
#include <iostream>
#include <sstream>
#include "Moose.h"

using namespace std;

/**
 * The Constructor.
 */
LibcurlUtils::LibcurlUtils() {

    //Create the curl handle
    curl = curl_easy_init();

    //Set the ignoreSslPeerVerification flag to false.
    ignoreSslPeerVerification = false;

}

/**
 * The Destructor.
 */
LibcurlUtils::~LibcurlUtils() {

    //Cleanup handle
    curl_easy_cleanup(curl);

    return;
}

/**
 * Uses libcurl and GET to return the contents located at url.
 *
 * @param url The URL of the GET request.
 * @return The contents at the URL or an error message if one took place.
 */
string LibcurlUtils::get(string url, string username, string password) {

    //If handle was successfully created
    if(curl) {

        //String to hold contents retrieved with GET.
        string buffer;

        //Reinitialize curl handle
        curl_easy_reset(curl);

        //Set the error buffer
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);

        //Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url.data());

        //Set the write callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeGetData);

        //Set the data pointer to write to the provided buffer
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        //If the ignoreSslPeerVerification flag is set to true
        if(ignoreSslPeerVerification) {

            //Do not verify server certificate
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

        }

        //Set the username and password
        curl_easy_setopt(curl, CURLOPT_USERNAME, username.data());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password.data());

        //Attempt to retrieve from the remote page
        result = curl_easy_perform(curl);

        //Check the result
        if (result == CURLE_OK) {

            //Create a variable to hold HTTP response code
            long http_code;

            //Call easy info function to get the code of the last call.
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

            //If the response code is other than 200 OK
            if(http_code != 200) {

                //Create a string to describe the error
                string error = "A GET request to ";
                stringstream ss;
                ss << http_code;
                error += url;
                error += " returned HTTP code ";
                error += ss.str();
                error += ".";

                //Return the error.
                return error ;

            }

            //Return the contents of the GET response
            return buffer;

        } else {
            //Return the curl error
            return string(error);
        }

    } else {

        //Return a CURL initialization error
        return "CURL could not be initialized.";
    }
}

/**
 * Uses libcurl and POST to transmit value at url.
 *
 * @param url The url that is used to post the value.
 * @param value The value that is posted to the url.
 * @return A string containing the error if one took place. Else returns an empty string.
 */
string LibcurlUtils::post(string url, string value, string username, string password) {

    //If handle was successfully created and the value is not empty
    if(curl && !value.empty()) {

        //Create string to post
        string data = "post=" + value;

        //Reinitialize curl handle
        curl_easy_reset(curl);

        //Set the POST field with data
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.data());

        //Set the error buffer
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);

        //Set the URL for the post
        curl_easy_setopt(curl, CURLOPT_URL, url.data());

        //If the ignoreSslPeerVerification flag is set to true
        if(ignoreSslPeerVerification) {

            //Do not verify server certificate
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

        }

        //Set the username and password
        curl_easy_setopt(curl, CURLOPT_USERNAME, username.data());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password.data());

        //Attempt to post to the remote page
        result = curl_easy_perform(curl);

        //Check the result
        if (result == CURLE_OK) {

            //Create a variable to hold HTTP response code
            long http_code;

            //Call easy info function to get the code of the last call.
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

            //If the response code is other than 200 OK
            if(http_code != 200) {

                //Create a string to describe the error
                string error = "A POST request to ";
                stringstream ss;
                ss << http_code;
                error += url;
                error += " returned HTTP code ";
                error += ss.str();
                error += ".";

                //Return the error.
                return error ;

            }

            //Return an empty string indicating no error
            return "";

        } else {

            //Return the curl error
            return string(error);
        }

    } else {

        //Return a CURL initialization error
        return "CURL could not be initialized.";
    }
}

/**
 * Sets the ignoreSslPeerVerification flag. If ignoreSslPeerVerification flag is
 * set to true then cURL will skip peer certificate verification for HTTPS urls.
 * This flag should only be set to true for testing purposes.
 *
 * @param ignoreSslPeerVerification The value for the ignoreSslPeerVerification flag.
 */
void LibcurlUtils::setIgnoreSslPeerVerification(bool ignoreSslPeerVerification) {

    //Set the instance value to the value in ignoreSslPeerVerification.
    this->ignoreSslPeerVerification = ignoreSslPeerVerification;

}

/**
 * A callback required by the C libcurl library to write the contents returned by get() to a buffer.
 *
 * @param data The get data.
 * @param size The size of each item.
 * @param nmemb The number of items in memory.
 * @param buffer The buffer to store the get() contents.
 * @return The amount written which should be size * nmemb.
 */
int LibcurlUtils::writeGetData(char * data,  size_t size, size_t nmemb, string buffer) {

    //Append the data to the buffer
    buffer.append(data, size * nmemb);

    //Compute the size written to buffer
    int result = size * nmemb;

    //Return the resulting size
    return result;
}
