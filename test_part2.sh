#!/bin/bash

echo "=== Part 2 Testing: DELETE and UPDATE ==="
echo ""

# Clean start
rm -f *.tab *.bin

echo "1. CREATE TABLE students"
./db "create table students (id int not null, name char(20), age int, gpa int)"

echo ""
echo "2. INSERT test data"
./db "insert into students values (1, 'Alice Johnson', 20, 85)"
./db "insert into students values (2, 'Bob Smith', 22, 90)"
./db "insert into students values (3, 'Charlie Brown', 21, 78)"
./db "insert into students values (4, 'Diana Prince', 23, 95)"
./db "insert into students values (5, 'Eve Anderson', 20, 88)"

echo ""
echo "3. SELECT all (initial data)"
./db "select * from students"

echo ""
echo "4. DELETE with WHERE (id = 3)"
./db "delete from students where id = 3"

echo ""
echo "5. SELECT after delete"
./db "select * from students"

echo ""
echo "6. UPDATE with WHERE (update age where id = 2)"
./db "update students set age = 25 where id = 2"

echo ""
echo "7. SELECT after update"
./db "select * from students"

echo ""
echo "8. UPDATE multiple rows (update gpa where age = 20)"
./db "update students set gpa = 90 where age = 20"

echo ""
echo "9. SELECT after multi-row update"
./db "select * from students"

echo ""
echo "10. DELETE with IS NULL test - create table with nulls"
./db "create table grades (id int, quiz int, final int)"
./db "insert into grades values (1, 85, 90)"
./db "insert into grades values (2, NULL, 88)"
./db "insert into grades values (3, 92, NULL)"
./db "select * from grades"

echo ""
echo "11. DELETE where quiz IS NULL"
./db "delete from grades where quiz is null"
./db "select * from grades"

echo ""
echo "12. Test DELETE all rows (no WHERE)"
./db "create table temp (x int)"
./db "insert into temp values (1)"
./db "insert into temp values (2)"
./db "insert into temp values (3)"
./db "select * from temp"
./db "delete from temp"
./db "select * from temp"

echo ""
echo "13. Test UPDATE with no matching rows (warning)"
./db "update students set name = 'Nobody' where id = 999"

echo ""
echo "14. Test DELETE with no matching rows (warning)"
./db "delete from students where id = 999"

echo ""
echo "15. Final state - SELECT all tables"
./db "list table"
echo ""
echo "Students table:"
./db "select * from students"
echo ""
echo "Grades table:"
./db "select * from grades"

echo ""
echo "=== Testing Complete ==="

