=====================
 XDIAG command notes
=====================

The ``++xdiag`` command can be used to test that individual lines are being asserted /
unasserted. The command takes two parameters: ``mode`` and ``value``.

To manipulate **control lines** use mode 1, and for data lines use mode 0.

To assert a **line/data bit**, simply speciy one of the values in the table below.

To assert multiple lines/bits simultaneously, simply add the values.

To un-assert a line, subtract its value from 255 The following two tables list the GPIB
signals and the command used to assert them.

Command signals:

:Assert IFC:    ``++xdiag 1 1``
:Assert NDAC:   ``++xdiag 1 2``
:Assert NRFD:   ``++xdiag 1 4``
:Assert DAV:    ``++xdiag 1 8``
:Assert EOI:    ``++xdiag 1 16``
:Assert REN:    ``++xdiag 1 32``
:Assert SRQ:    ``++xdiag 1 64``
:Assert ATN:    ``++xdiag 1 128``
:Assert ALL:    ``++xdiag 1 255``
:Un-assert ALL: ``++xdiag 1 0``

Data bits:

:Assert DA01:   ``++xdiag 0 1``
:Assert DA02:   ``++xdiag 0 2``
:Assert DA03:   ``++xdiag 0 4``
:Assert DA04:   ``++xdiag 0 8``
:Assert DA05:   ``++xdiag 0 16``
:Assert DA06:   ``++xdiag 0 32``
:Assert DA07:   ``++xdiag 0 64``
:Assert DA08:   ``++xdiag 0 128``
:Assert ALL:    ``++xdiag 0 255``
:Un-assert ALL: ``++xdiag 0 0``
