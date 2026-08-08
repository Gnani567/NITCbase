# NITCbase

NITCbase is a Database Management System (DBMS) project developed in C++ to explore and implement the fundamental components involved in building a database system.

The project focuses on understanding how data is stored, accessed, managed, and processed internally rather than relying entirely on an existing database engine.

## Overview

NITCbase is organized into multiple components that work together to provide the basic infrastructure required by a database management system.

The project currently includes components related to:

- Disk and block management
- Buffer management
- Cache management
- B+ Tree indexing
- Block access
- Relational algebra
- Schema management
- Database frontend and query interface

The project is being developed incrementally, with individual DBMS components implemented and integrated as the system evolves.

## Project Structure

```text
NITCbase/
├── Disk/
├── Files/
├── mynitcbase/
│   ├── Algebra/
│   ├── BlockAccess/
│   ├── BPlusTree/
│   ├── Buffer/
│   ├── Cache/
│   ├── Disk_Class/
│   ├── Frontend/
│   ├── FrontendInterface/
│   └── Schema/
├── XFS_Interface/
├── main.cpp
├── Makefile
└── README.md
