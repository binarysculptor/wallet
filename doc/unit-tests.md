Compiling/running unit tests
------------------------------------

Unit tests will be automatically compiled if dependencies were met in configure
and tests weren't explicitly disabled.

After configuring, they can be run with 'make check'.

To run the libertyd tests manually, launch src/test/test_liberty .

To add more libertyd tests, add `BOOST_AUTO_TEST_CASE` functions to the existing
.cpp files in the test/ directory or add new .cpp files that
implement new BOOST_AUTO_TEST_SUITE sections.

To run the liberty-qt tests manually, launch src/qt/test/liberty-qt_test

To add more liberty-qt tests, add them to the `src/qt/test/` directory and
the `src/qt/test/test_main.cpp` file.
