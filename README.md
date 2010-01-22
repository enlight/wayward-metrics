Wayward Metrics
=================

Overview
--------

This software consists of 3 parts:

   * Reporting agents which feed data into the aggregation server.
   * The aggregation server gathers data from the reporting agents,
     archives it to disk and allows other clients to observe the
     event streams being reported as well as retrieve older event
     streams.
   * Analysis clients observe and retrieve event stream data from
     the aggregation server and perform analytical operations as
     well as visualization of the data.  Analysis clients can combine
     data from multiple reporting sessions as well to get a broader
     and more complete view in a networked or distributed environment.

The data which can be tracked by this system is easily extended, but the
initial versions of the software will support:

   * Logging
   * Threading and Locking
   * Basic Profiling by manual instrumentation
   * Monitoring job / task queues (start / stop times).

Future extensions may include:

   * Tracking memory allocations, including support for arenas, multiple
     heaps and garbage collection.
   * Tracking file system I/O.
   * Hooking into other providers like dtrace and ptrace/truss and
     bridging that data into this system for further analysis.
   * Database activity (queries, operations).

All records include the thread ID that was active as well as high precision
time stamps.

Licensing Model
---------------

This is still being worked out. It is likely that the reporting client and
aggregation server will be under either the MIT or Apache 2 licenses. The
licensing for the graphical analysis client remains to be determined.
Text-based analysis tools will be provided under the same license as the
reporting client and aggregation server.

Status
------

This project is in the very early stages of development and is not ready
for usage at this time.

Implementations
---------------

The various implementations for each language are in subdirectories here
and have their own README outlining their dependencies and how to build
them.

Currently, we have a prototype implementation in Python.  Once the basics
are working in Python, we will provide a C / C++ reporting library.

