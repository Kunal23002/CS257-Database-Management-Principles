#!/bin/bash

echo "=== Part 2 Checklist Testing ==="
echo ""

# Clean start
rm -f *.tab *.bin

echo "01. Create table, insert data, test SELECT *, single column, multi-column"
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

echo ""
echo "Test SELECT *:"
./db "select * from class"

echo ""
echo "Test SELECT single column (Student_Name):"
./db "select Student_Name from class"

echo ""
echo "Test SELECT multi-column (Student_Name, Total):"
./db "select Student_Name, Total from class"

echo ""
echo "02. Test single row delete"
./db "insert into class values ('Bad_Student', 'M', 50, 100, 150)"
./db "delete from class where Student_Name = 'Bad_Student'"

echo ""
echo "03. Test delete where no rows found"
./db "delete from class where Student_Name = 'NonExistent'"

echo ""
echo "04. Test multi-row delete (Total < 370)"
./db "delete from class where Total < 370"
./db "select * from class"

echo ""
echo "05. Test single row update"
./db "insert into class values ('David', 'M', 80, 320, 400)"
./db "update class set Quiz_Total = 350 where Student_Name = 'David'"
./db "select Student_Name, Quiz_Total from class where Student_Name = 'David'"

echo ""
echo "06. Test update when no rows found"
./db "update class set Quiz_Total = 999 where Student_Name = 'Nobody'"

echo ""
echo "07. Test multi-row update (Quiz_Total where Quiz_Total > 350)"
./db "update class set Quiz_Total = 350 where Quiz_Total > 350"
./db "select Student_Name, Quiz_Total from class"

echo ""
echo "08. Test SELECT with WHERE single condition"
./db "select * from class where Total > 450"

echo ""
echo "09. Test SELECT with WHERE case sensitive (Student_Name < 'Grace')"
./db "select * from class where Student_Name < 'Grace'"

echo ""
echo "10. Test SELECT with WHERE for NULL columns"
./db "create table test_null(id int, value int)"
./db "insert into test_null values (1, 100)"
./db "insert into test_null values (2, NULL)"
./db "insert into test_null values (3, 200)"
./db "select * from test_null where value is null"
./db "select * from test_null where value is not null"

echo ""
echo "11. Test SELECT with AND"
./db "select * from class where Total > 400 and Gender = 'F'"

echo ""
echo "12. Test SELECT with OR"
./db "select * from class where Student_Name = 'Alice' or Student_Name = 'David'"

echo ""
echo "13. Test SELECT with ORDER BY"
./db "select * from class order by Total"

echo ""
echo "14. Test SELECT with WHERE and ORDER BY"
./db "select * from class where Gender = 'F' order by Total"

echo ""
echo "15. Test SELECT SUM()"
./db "select sum(Total) from class"

echo ""
echo "16. Test SELECT SUM() with WHERE"
./db "select sum(Total) from class where Gender = 'F'"

echo ""
echo "17. Test SELECT AVG()"
./db "select avg(Total) from class"

echo ""
echo "18. Test SELECT AVG() with WHERE"
./db "select avg(Exams) from class where Gender = 'M'"

echo ""
echo "19. Test SELECT COUNT()"
./db "select count(Student_Name) from class"

echo ""
echo "20. Test SELECT COUNT() with WHERE"
./db "select count(*) from class where Total > 420"

echo ""
echo "21. Test SUM(), AVG() with NULLs"
./db "select sum(value) from test_null"
./db "select avg(value) from test_null"

echo ""
echo "22. Test COUNT(*), COUNT(column) with NULLs"
./db "select count(*) from test_null"
./db "select count(value) from test_null"

echo ""
echo "=== All tests complete ==="

