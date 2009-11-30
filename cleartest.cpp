/**
 * OpenGL ES 2.0 memory performance estimator
 * Copyright (C) 2009 Nokia
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
 * Clear test
 */
#include "cleartest.h"
#include <GLES2/gl2.h>

ClearTest::ClearTest()
{
    glClearColor(1.0f, 0.27f, 0.0f, 1.0f);
}

void ClearTest::operator()(int frame)
{
    glClear(GL_COLOR_BUFFER_BIT);
}

std::string ClearTest::name() const
{
    return "clear";
}
