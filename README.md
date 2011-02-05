	
Eon3D - A simplistic 3D software renderer
=========================================

(C) 2010-2011 Francesco Romani < fromani | gmail : com >


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

Eon3D is a simplistic 3D software rendered inspired by (now abandoned?)
Plush3D by Nullsoft.
Eon3D aims for simplicity, performance (for a software renderer) and
compactness, in this order.
Eon3D is composed by a set of libraries which composes the core software
and a kit of companion tools exploiting those libraries.
Other key aspects of the software are:

* compactness. Bloat should be trimmed as most as is possible.
* minimal dependencies set. If the dependency license allow that and if
  it is convenient, the dependency is bundled with the software.
* ease of embeddability (if you care).


License Notice
--------------

The core renderer is licensed under a ZLIB-like license,
as for plush3D (see COPYING).
The AVI output module uses (and embeds) AVILIB, which is released under
the GPL2 license. If you enable the AVI output module, the license of
Eon3D will thus become GPL2.


Requirements/Dependencies
-------------------------

* POSIX platform (libC std + stdint/inttypes is enough, actually).
* C99 compiler (tested against gcc 4+).
* SDL for graphical console.
* libPNG for PNG output.

### EOF ###

