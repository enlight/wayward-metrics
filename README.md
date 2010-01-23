Wayward Metrics
=================

Wayward Metrics is a temporary name.

Overview
--------

Wayward Metrics is a solution for collecting, storing and analyzing
data logged by an application, whether that application runs
as a single process or as part of a distributed cluster.

The data which can be tracked by this system is easily extended, but the
initial versions of the software will support:

   * Logging
   * Threading and Locking
   * Basic Profiling by manual instrumentation
   * Monitoring job / task queues (start / stop times).

Future extensions will include:

   * Tracking memory allocations, including support for arenas, multiple
     heaps and garbage collection.
   * Tracking file system I/O.
   * Hooking into other providers like dtrace and ptrace/truss and
     bridging that data into this system for further analysis.
   * Database activity (queries, operations).

All records include the thread ID that was active as well as high precision
time stamps.

The idea is that you should be able to instrument your application
or framework so that the data can be used to determine what was going
on within the application for debugging or performance optimization.
By extending the system, you could even use this software for analyzing
interactions with the user interface, mean time between failure for your
application in production, order / sales data, or whatever your
imagination comes up with.

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
     An example of an analysis client could be a tool that analyzes
     locking / threading behavior to proactively identify buggy
     behavior.

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

Inspiration
-----------

My work on this has been inspired by other software that I have used
or admired in the past.

This includes:

   * [Haskell ThreadScope](http://code.haskell.org/ThreadScope/)
   * [GC Spy](http://www.cs.kent.ac.uk/projects/gc/gcspy/) ([and link at Sun](http://research.sun.com/projects/gcspy/))
   * [Deja Insight](http://www.dejatools.com/dejainsight)
   * [Google Speed Tracer](http://code.google.com/webtoolkit/speedtracer/)
   * [the net panel in Firebug](http://getfirebug.com/net.html)
   * [Tableau](http://www.tableausoftware.com/)

