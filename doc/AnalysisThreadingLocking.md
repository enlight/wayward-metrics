Threading and Locking Analysis
==============================

Requirements
------------

Analysis of locking behavior should include, at a minimum, functionality similar to these tools:

   * [Linux lockdep](http://www.mjmwired.net/kernel/Documentation/lockdep-design.txt)
   * [Linux lockstat](http://www.mjmwired.net/kernel/Documentation/lockstat.txt)
   * [Multi-Threaded Analyzer](http://i1.dk/mta/mta.html)
   * [locktrace from AIX](http://publib.boulder.ibm.com/infocenter/aix/v6r1/index.jsp?topic=/com.ibm.aix.cmds/doc/aixcmds3/locktrace.htm)
   * [splat from AIX](http://publib.boulder.ibm.com/infocenter/aix/v6r1/index.jsp?topic=/com.ibm.aix.prftools/doc/prftools/prftools05.htm)

Visualization
-------------

Unlike the above tools, we also want to provide a graphical view of threading
activity. This should be able to present graphs of activity per-thread, showing
lock activity (and contention) in a color-coded fashion.

Inspiration can be obtained from tools such as these:

   * [Haskell ThreadScope](http://code.haskell.org/ThreadScope/)
   * [Analysis of the GIL in Python](http://www.dabeaz.com/blog/2010/01/python-gil-visualized.html)

