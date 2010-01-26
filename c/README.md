Wayward Metrics - C
===================

Platform Support
----------------

Currently, this is only tested to build on MacOS X. In the future, we will support,
at a minimum, these platforms:

   * Windows
   * Mac OS X
   * Linux

Build
-----

   * Install CMake. This has been tested with 2.8, but may work with older versions.
   * Generate the projects or Makefile with CMake.
   * Build.

Usage
-----

Use the reporting interface, as found in wayward/metrics/reporting/base.h.  Auxillary
reporting functionality can be found in other headers in the same directory.

Installation
------------

   * Who knows?

Known Issues
------------

   * The implementation is using blocking I/O. This is hidden within the
     message_queue implementation on a background thread, so it should
     not impact your application.

Why C?
------

The base library and functionality is entirely written in C (rather than C++). The
reasoning behind this decision was:

   * We want this to be widely used by various open source applications, frameworks,
     and libraries. Many of these are in C. (Apache, ab, Python, Postgres, etc).

