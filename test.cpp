/**
 * OpenGL ES 2.0 memory performance estimator
 * Copyright (C) 2010 Nokia
 *
 * \author Sami Kyöstilä <sami.kyostila@nokia.com>
 *
 * Test base class
 */
#include "test.h"
#include <stdexcept>

void Test::fail(const std::string& reason)
{
    throw std::runtime_error(reason);
}
