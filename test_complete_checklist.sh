#!/bin/bash

echo "=== COMPLETE CHECKLIST VERIFICATION ==="
echo ""
rm -f *.tab *.bin

echo "============================================"
echo "NORMAL TEST SCENARIOS (01-22)"
echo "============================================"

echo ""
echo "01. Create table, insert 15 rows, test SELECT variations"
./db "create table class(Student_Name char(20) NOT NULL, Gender char(1), Exams int, Quiz_Total int, Total int NOT NULL)"
./db "insert into class values ('Alice', 'F', 85, 320, 405)"
./db "insert into class values ('Bob', 'M', 90, 350, 440)"
./db "insert into class values ('Charlie', 'M', 78, 310, 388)"
./db "insert into class values ('Diana', 'F', 95, 380, 475)"
./db "insert into class values ('Eve', 'F', 88, 330, 418)"
./db "insert into class values ('Frank', 'M', 82, 300, 382)"
./db "insert into class values ('Grace', 'F', 91, 360, 451)"
./db "insert into class values ('Henry', 'M', 75, 280, 355)"
./db "insert into class values ('Ivy', 'F', 87, 340, 427)"
./db "insert into class values ('Jack', 'M', 93, 370, 463)"
./db "insert into class values ('Kelly', 'F', 89, 345, 434)"
./db "insert into class values ('Leo', 'M', 81, 295, 376)"
./db "insert into class values ('Mia', 'F', 94, 375, 469)"
./db "insert into class values ('Noah', 'M', 77, 285, 362)"
./db "insert into class values ('Olivia', 'F', 92, 365, 457)"

echo "SELECT * FROM class:"
./db "select * from class" | tail -5

echo ""
echo "SELECT Student_Name FROM class:"
./db "select Student_Name from class" | tail -5

echo ""
echo "SELECT Student_Name, Total FROM class:"
./db "select Student_Name, Total from class" | tail -5

echo ""
echo "02. Single row delete"
./db "insert into class values ('Bad_Student', 'M', 50, 100, 150)"
./db "delete from class where Student_Name = 'Bad_Student'" | grep "row(s) deleted"

echo ""
echo "03. Delete with no rows found"
./db "delete from class where Student_Name = 'NonExistent'" | grep "Warning"

echo ""
echo "04. Multi-row delete (3 rows)"
./db "delete from class where Total < 370" | grep "row(s) deleted"

echo ""
echo "05. Single row update"
./db "insert into class values ('David', 'M', 80, 320, 400)"
./db "update class set Quiz_Total = 350 where Student_Name = 'David'" | grep "row(s) updated"

echo ""
echo "06. Update with no rows found"
./db "update class set Quiz_Total = 999 where Student_Name = 'Nobody'" | grep "Warning"

echo ""
echo "07. Multi-row update (4 rows)"
./db "update class set Quiz_Total = 350 where Quiz_Total > 350" | grep "row(s) updated"

echo ""
echo "08. SELECT with WHERE single condition"
./db "select * from class where Total > 450" | tail -3

echo ""
echo "09. SELECT with WHERE case sensitive (Student_Name < 'Grace')"
./db "select * from class where Student_Name < 'Grace'" | tail -3

echo ""
echo "10. SELECT with WHERE for NULL/NOT NULL"
./db "create table test_null(id int, value int)"
./db "insert into test_null values (1, 100)"
./db "insert into test_null values (2, NULL)"
./db "insert into test_null values (3, 200)"
./db "select * from test_null where value is null" | tail -3
./db "select * from test_null where value is not null" | tail -3

echo ""
echo "11. SELECT with AND"
./db "select * from class where Total > 400 and Gender = 'F'" | tail -3

echo ""
echo "12. SELECT with OR"
./db "select * from class where Student_Name = 'Alice' or Student_Name = 'David'" | tail -3

echo ""
echo "13. SELECT with ORDER BY"
./db "select * from class order by Total" | tail -3

echo ""
echo "14. SELECT with WHERE and ORDER BY"
./db "select * from class where Gender = 'F' order by Total" | tail -3

echo ""
echo "15. SELECT SUM()"
./db "select sum(Total) from class" | grep -A1 "SUM"

echo ""
echo "16. SELECT SUM() with WHERE"
./db "select sum(Total) from class where Gender = 'F'" | grep -A1 "SUM"

echo ""
echo "17. SELECT AVG()"
./db "select avg(Total) from class" | grep -A1 "AVG"

echo ""
echo "18. SELECT AVG() with WHERE"
./db "select avg(Exams) from class where Gender = 'M'" | grep -A1 "AVG"

echo ""
echo "19. SELECT COUNT()"
./db "select count(Student_Name) from class" | grep -A1 "COUNT"

echo ""
echo "20. SELECT COUNT() with WHERE"
./db "select count(*) from class where Total > 420" | grep -A1 "COUNT"

echo ""
echo "21. SUM(), AVG() with NULLs"
./db "select sum(value) from test_null" | grep -A1 "SUM"
./db "select avg(value) from test_null" | grep -A1 "AVG"

echo ""
echo "22. COUNT(*), COUNT(column) with NULLs"
./db "select count(*) from test_null" | grep -A1 "COUNT"
./db "select count(value) from test_null" | grep -A1 "COUNT"

echo ""
echo "============================================"
echo "NATURAL JOIN TESTS (Requirement from line 51)"
echo "============================================"

echo ""
echo "Setup for NATURAL JOIN tests"
./db "create table students(name char(20), ID int, gender char(1))"
./db "create table grades(ID int, quiz int, final int)"
./db "insert into students values ('John', 1001, 'M')"
./db "insert into students values ('Jane', 1002, 'F')"
./db "insert into grades values (1001, 85, 90)"
./db "insert into grades values (1002, 88, 92)"

echo ""
echo "Test: SELECT * FROM students NATURAL JOIN grades"
./db "select * from students natural join grades" | tail -4

echo ""
echo "============================================"
echo "ERROR TEST SCENARIOS (23-31)"
echo "============================================"

echo ""
echo "23. Syntax errors in DELETE (should not crash)"
./db "delete from" 2>&1 | head -1
./db "delete class where x = 1" 2>&1 | head -1
./db "delete from class where" 2>&1 | head -1

echo ""
echo "24. Syntax errors in UPDATE (should not crash)"
./db "update class" 2>&1 | head -1
./db "update class set" 2>&1 | head -1
./db "update class set x" 2>&1 | head -1

echo ""
echo "25. Syntax errors in SELECT (should not crash)"
./db "select from class" 2>&1 | head -1
./db "select * from" 2>&1 | head -1
./db "select * from class where" 2>&1 | head -1

echo ""
echo "26. Data type mismatch on INSERT"
./db "insert into class values ('Test', 'abc', 90, 300, 400)" 2>&1 | head -1

echo ""
echo "27. NOT NULL enforcement on INSERT"
./db "insert into class values (NULL, 'M', 90, 300, 400)" 2>&1 | head -1

echo ""
echo "28. NOT NULL enforcement on UPDATE"
./db "update class set Student_Name = NULL where Gender = 'M'" 2>&1 | head -1

echo ""
echo "29. Type mismatch in WHERE (should handle gracefully)"
./db "select * from class where Total = 'abc'" | tail -2

echo ""
echo "30. Invalid data value (should catch)"
./db "insert into class values ('Test', 'M', 999999999999999999, 300, 400)" 2>&1 | head -1

echo ""
echo "31. Invalid relational operator / aggregate parameter"
./db "select sum(Student_Name) from class" 2>&1 | head -1

echo ""
echo "============================================"
echo "VERIFICATION COMPLETE"
echo "============================================"

