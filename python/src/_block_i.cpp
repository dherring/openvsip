//
// Copyright (c) 2014 Stefan Seefeld
// All rights reserved.
//
// This file is part of OpenVSIP. It is made available under the
// license contained in the accompanying LICENSE.BSD file.

#include "block_api.hpp"

BOOST_PYTHON_MODULE(_block_i)
{
  using namespace pyvsip;
  initialize();
  define_block<1, int>("Block1");
  define_block<2, int>("Block2");
}