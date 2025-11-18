rm *.tab
rm *.bin
./db "create table students(name char(20), ID int, gender char(1), scores int)"
./db "insert into students values ('John F Kennedy', 10001, 'M', 99)"
./db "insert into students values ('Bill Clinton', 10002, 'M', 85)"
./db "insert into students values ('Donald Trump', 10003, 'M', 95)"
./db "select * from students"

./db "create table grades(ID int, quiz int, final int)"
./db "insert into grades values (10001, 70, 71)"
./db "insert into grades values (10002, 80, 81)"
./db "insert into grades values (10001, 80, 81)"
./db "insert into grades values (10005, 90, 91)"
./db "select * from grades"

./db "select * from students natural join grades"
