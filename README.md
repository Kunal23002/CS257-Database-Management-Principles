# Database Project - Part 1

A simple database management system implementation supporting SQL-like commands for table creation, data manipulation, and querying.

## Overview

This project implements a basic database system that supports:
- Table creation and management (CREATE TABLE, DROP TABLE, LIST TABLE)
- Data insertion with NOT NULL constraint enforcement
- Query operations (SELECT with formatted output)
- Natural joins between tables
- Schema listing and reporting

## Building

### Prerequisites
- C++ compiler with C++17 support (g++ or clang++)
- Standard C++ library

### Compilation

```bash
g++ -std=c++17 -O2 db.cpp -o db
```

Or with clang++:
```bash
clang++ -std=c++17 -O2 db.cpp -o db
```

## Usage

The database program takes a single SQL command as a command-line argument:

```bash
./db "SQL_COMMAND"
```

### Examples

**Create a table:**
```bash
./db "create table students (id int not null, name char(20), age int)"
```

**Insert data:**
```bash
./db "insert into students values (1, 'John Doe', 20)"
./db "insert into students values (2, 'Jane Smith', 22)"
```

**Query all records:**
```bash
./db "select * from students"
```

**Natural join:**
```bash
./db "select * from students natural join grades"
```

**List all tables:**
```bash
./db "list table"
```

**List schema for a table:**
```bash
./db "list schema for students"
```

**List schema to file:**
```bash
./db "list schema for students to report.txt"
```

**Drop a table:**
```bash
./db "drop table students"
```

## Supported SQL Statements

### Data Definition Language (DDL)

#### CREATE TABLE
Creates a new table with specified columns.

**Syntax:**
```sql
CREATE TABLE table_name (
    column_name TYPE [NOT NULL],
    ...
)
```

**Supported Types:**
- `int` - Integer (4 bytes)
- `char(n)` - Fixed-length character string of length n

**Example:**
```sql
CREATE TABLE students (
    id int not null,
    name char(20),
    age int
)
```

#### DROP TABLE
Removes a table and its data file from the database.

**Syntax:**
```sql
DROP TABLE table_name
```

#### LIST TABLE
Lists all tables in the database.

**Syntax:**
```sql
LIST TABLE
```

#### LIST SCHEMA
Displays or saves the schema (table structure) for a specific table.

**Syntax:**
```sql
LIST SCHEMA FOR table_name [TO filename]
```

### Data Manipulation Language (DML)

#### INSERT
Inserts a new row into a table.

**Syntax:**
```sql
INSERT INTO table_name VALUES (value1, value2, ...)
```

**Supported Values:**
- Integer literals (e.g., `100`, `42`)
- String literals (e.g., `'John Doe'`, `'Hello'`)
- `NULL` keyword

**NOT NULL Constraint:**
- If a column is defined as `NOT NULL`, attempting to insert `NULL` will result in an error.

**Example:**
```sql
INSERT INTO students VALUES (1, 'John F Kennedy', 20)
INSERT INTO students VALUES (NULL, 'Jane Doe', 25)  -- Error if id is NOT NULL
```

#### SELECT
Queries data from one or more tables.

**Simple SELECT:**
```sql
SELECT * FROM table_name
```

**Natural Join:**
```sql
SELECT * FROM table1 NATURAL JOIN table2
```

The natural join automatically matches rows where common columns (same name and type) have equal values. Output columns are ordered as:
1. Common columns (appears once)
2. Unique columns from table1
3. Unique columns from table2

**Output Format:**
- Column headers (left-aligned)
- Separator line with dashes
- Data rows (integers right-aligned, strings left-aligned)
- Record count at the end

**Example Output:**
```
name              ID gender scores
-------------- ----- ------ ------
John F Kennedy 10001 M          99
Bill Clinton   10002 M          85

 2 record(s) selected.
```

## File Structure

### Data Files

- **`dbfile.bin`** - Catalog file containing table metadata (TPDs - Table Packed Descriptors)
- **`<table_name>.tab`** - Data file for each table containing:
  - File header (record size, count, offsets)
  - Record data (fixed-length records with 4-byte alignment)

### Source Files

- **`db.cpp`** - Main implementation file containing:
  - Lexical analyzer (tokenizer)
  - Semantic analyzer
  - SQL statement handlers
  - Table file operations
  - Record packing/unpacking
  
- **`db.h`** - Header file containing:
  - Data structures (TPD, CD entries, token lists)
  - Function declarations
  - Constants and enums

## Record Format

Each record in a table file is stored with the following format:
- 1-byte length tag per column (0 for NULL)
- Payload:
  - `int`: 4-byte integer value
  - `char(n)`: n-byte character string (padded with nulls if shorter)
- Records are padded to 4-byte boundaries

## Features

### Part 1 Features

✅ CREATE TABLE - Define table structure with columns and data types  
✅ DROP TABLE - Remove tables from the database  
✅ INSERT - Add rows with NOT NULL constraint checking  
✅ SELECT - Query with formatted tabular output  
✅ NATURAL JOIN - Join tables on matching column names and values  
✅ LIST TABLE - List all tables in the database  
✅ LIST SCHEMA - Display or save table schema information  

### Constraint Support

- **NOT NULL** - Enforced during INSERT operations
- Column name uniqueness within a table
- Table name uniqueness in the database

## Error Handling

The system returns appropriate error codes for various conditions:
- `INVALID_TABLE_NAME` - Table doesn't exist or invalid name
- `DUPLICATE_TABLE_NAME` - Table already exists
- `DUPLICATE_COLUMN_NAME` - Column name repeated in table definition
- `INVALID_COLUMN_DEFINITION` - Attempted NULL in NOT NULL column
- `INVALID_STATEMENT` - Syntax error in SQL command
- And more...

## Testing

A test script `join.sh` is provided that exercises various features:

```bash
bash join.sh
```

This creates tables, inserts data, performs queries and joins, demonstrating the full functionality.

## Limitations

- Maximum identifier length: 16 characters
- Maximum columns per table: 16
- Maximum token length: 32 characters
- Case-insensitive table names (for lookups)
- Case-sensitive column names
- No WHERE clause filtering
- No ORDER BY sorting
- No UPDATE or DELETE statements (Part 2)

## Development Notes

- The catalog (`dbfile.bin`) is updated atomically on disk
- Table file headers maintain transient in-memory pointers to TPD entries
- All file operations use binary mode for portability
- Record sizes are automatically calculated and 4-byte aligned

## Author

Kunal

## License

This is an academic project implementation.

