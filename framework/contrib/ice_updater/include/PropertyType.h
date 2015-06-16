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

#ifndef PROPERTYTYPE_H
#define PROPERTYTYPE_H

/**
 * The PropertyType enumeration consists of literals representing each type of
 * configuration property used by Updater.
 */
enum PropertyType {

    /**
     * This literal indicates a url.
     */
    URL = 0,

    /**
     * This literal indicates a simulation item id.
     */
    ITEM_ID,

    /**
     * This literal indicates a unique client key.
     */
    CLIENT_KEY,

    /**
     * This literal represents the username that should be used for
     * authenticating with the server.
     */
    USERNAME,

    /**
     * This literal represents the username that should be used for
     * authenticating with the server.
     */
    PASSWORD

};

#endif
