//
// Copyright 2019 Ettus Research, a National Instruments Brand
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include <uhdlib/utils/isatty.hpp>
#include <stdio.h>
#include <boost/test/unit_test.hpp>
#include <iostream>

BOOST_AUTO_TEST_CASE(test_isatty)
{
    // We can't really pass or fail based on the result, because it depends on
    // how the tests are executed. We'll just run it and see it doesn't crash.
    if (uhd::is_a_tty(2)) {
        std::cout << "stderr is a TTY" << std::endl;
    } else {
        std::cout << "stderr is not a TTY" << std::endl;
    }

    FILE* tmp_file = std::tmpfile();
#ifdef UHD_PLATFORM_WIN32
    BOOST_REQUIRE(!uhd::is_a_tty(_fileno(tmp_file)));
#elif _POSIX_C_SOURCE >= _200112L
    BOOST_REQUIRE(!uhd::is_a_tty(fileno(tmp_file)));
#else
    // I got 99 problems but dealing with portability ain't one
    BOOST_REQUIRE(!uhd::is_a_tty(99));
#endif
}
