[![Build Status](https://travis-ci.org/usox/harm.svg?branch=master)](https://travis-ci.org/usox/harm)

HaRM - Hack Relation Mapper
===========================

Goal
----

The main purpose of HaRM is to perform simple CRUD-Operations in hack strict
mode - that's it.

Usage
-----

- Create a .harm file containing the database structure (see `tests/sample.harm` for a
boilerplate).
- Generate interface `./bin/harmgen interface <path_to_your_harm_file> > MyTableInterface.hh`
- Generate table `./bin/harmgen table <path_to_your_harm_file> > MyTable.hh`
- Use your existing PDO instance for the database connection

See the generated interface for further information.
