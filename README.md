HaRM - Hack Relation Mapper
===========================

Warning
-------

This library is currently under heavy development and should only be used for
demonstration purposes.

Goal
----

The main purpose of HaRM is to perform simple CRUD-Operations in hack strict
mode - nothing more, nothing less.

Usage
-----

- Create a .harm file containing the database structure (see `tests/sample.harm` for a
boilerplate).
- Generate interface `./bin/harmgen interface <path_to_your_pobj_file> > MyTableInterface.hh`
- Generate table `./bin/harmgen table <path_to_your_pobj_file> > MyTable.hh`

See the generated interface for further information.
