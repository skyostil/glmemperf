/**
 * OpenGL ES 2.0 memory performance estimator
 * Copyright (C) 2010 Nokia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * \author Sami Kyöstilä <sami.kyostila@nokia.com>
 *
 * Test base class
 */
#ifndef TEST_H
#define TEST_H

#include <string>

class Test
{
public:
    explicit Test()
    {
    }

    virtual ~Test()
    {
    }

    /**
     *  Make the test ready to be executed
     */
    virtual void prepare()
    {
    }

    /**
     *  Render a single frame of the test
     *  
     *  @param frame            Frame number
     */
    virtual void operator()(int frame) = 0;


    /**
     *  Release any resources allocated by prepare()
     */
    virtual void teardown()
    {
    }

    /**
     *  @returns a name for the test
     */
    virtual std::string name() const = 0;

protected:
    /**
     *  Abort a test
     */
    void fail(const std::string& reason);
};

#endif // TEST_H
