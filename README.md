
Eon3D - A simplistic 3D software renderer
=========================================

(C) 2010-2013 Francesco Romani < fromani | gmail : com >



	             __        __
	           _- /        \ -_
	          / //   _--------.\
	         /   |  /6666~~~~~~\\
	        |  | | (6666~~~@~~@||
	        |    |  \666~~//|\\\| |
	        |  \ \   \666\|||/\\\_/
	        |     \ /   /|//_/|/__
	        |  |\ \/    \_\ \|\|
	        _\__\_|   |  | ___ | ||
	       |\     |   |  |-   -| ||
	       | \     |   \ \____ \ \____
	       |  \    |    \_____\-_-__  \
	       |   \    |    // , )| \ )///
	       |    \    \  ,/ / /JJ |/JJJ
	       |     \    \   / /__- /    \
	       |      \    \/(  \ (  \     \
	       |       \______\  \_\  \_____\
	       |       |       \  \ \  \    | That is not lost which
	       |       |       |||| ||||    | can eternal lie,
	       |       |       \||| \|||    | And with strange aeons
	        \      |        JJJ  JJJ    | even that may render.
	         ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Overview
--------

Eon3D is a simplistic 3D software rendered derived by (now abandoned?)
Plush3D by Nullsoft.

Eon3D is an exploration and learning project on the branch of the 3D
graphics; so, the maximum performance, and the implementation of the
fancier algorythms are not the top priorities.
For the very nature of the project, the code will not be ever
GPU-accelerated.

The objectives for Eon3D are:

* simplicity: the code should be easy to read, easy to maintain and
  easy to keep in mind.
* compactness. Bloat should be trimmed as most as is possible.
* minimal dependencies set. If the dependency license allow that and if
  it is convenient, the dependency is bundled with the software.
* ease of embeddability (if you care).
* performance (for being a software renderer and a CPU-only program!)

Eon3D is made by a set of libraries which composes the core software
and a kit of companion tools exploiting those libraries.
Supporting tools (e.g. 3D model viewers) are provided as clients,
examples and debugging tools.


Requirements/Dependencies (common)
----------------------------------

* POSIX platform (libc std + stdint/inttypes is enough, actually).
* C99 compiler (tested against gcc 4+).
* [cmake](http://www.cmake.org)


Components
----------

The modules breakdown of Eon3D follows.

* core
  - License: ZLIB;
  - Dependencies: None;
  - Description: The core renderer library.

* console
  - License: ZLIB;
  - Dependencies: libSDL (LGPL, not bundled);
  - Description: Simple graphical I/O console.

* model
  - License: ZLIB;
  - Dependencies: rply (MIT, bundled);
  - Description: Simple model file loader.

* png (planned)
  - License: ZLIB;
  - Status: Planned;
  - Dependencies: libPNG (not bundled);
  - Description: Saves frame to PNG images.

* avi (planned)
  - License: GPL2;
  - Dependencies: avilib (bundled);
  - Description: Save renders to raw AVI files.


Getting Started
---------------

Assumptions: you got a unix-like (linux) shell running
and you are positioned into the eon3d source tree root.
`$` represents the command prompt. The eon3d source tree root
it is supposed to be named `eon3d`.

To build the software. 
* in-tree builds are supported, but out of tree builds are recommended.
* Create the binaries dir: `mkdir ../build_eon3d`.
* Then move into it: `cd ../build_eon3d`.
* Run cmake to configure the build environment: `cmake ../eon3d`.
* If you need to specify some configuration options, you
  can check out the 
  [cmake docs](http://www.cmake.org/cmake/help/runningcmake.html).
* Now you can build the package: `make`.
* If you want to install the software: `make install`.

Once Eon3D is built, you can start playing with the runnable `examples`
(check the corresponding subdirectory).

Eon3D comes with a (growing) set of tests. To build and run then,
you need the [check package](http://check.sf.net) and [python >= 2.5](http://www.python.org).
Python is already bundled in any {recent,decent} unix-like system.
Then you must explicitely enable the test suite at the configuration
step by modifying the above command in `cmake -DENABLE_TESTS=1 ../eon3d`.
The tests will now be built and they can be run.

License Notice
--------------

The core renderer is licensed under a ZLIB-like license, as for
plush3D (see LICENSE).
Every component has a license notification on top of the corresponding
source file. Most of the extensions modules adopt the same (ZLIB)
license. However If a module constitute an exception, that will be 
explicitly and plainly stated.
The AVI output module uses (and embeds) AVILIB, which is released under
the GPL2 license. If you enable the AVI output module, the license of
Eon3D will thus become GPL2.

TODO
----

* console: more input handling, logging, timestamping.
* core: truecolor handling, drop the palette

### EOF ###

