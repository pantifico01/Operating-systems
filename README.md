# Operating-systems
Лаба 1:
файлы: child1.c child2.c parent.c
компиляция
gcc -o parent parent.c
gcc -o child1 child1.c
gcc -o child2 child2.c

запуск 
./parent

условие: Вариант 20) Правило фильтрации: строки длины больше 10 символов отправляются в pipe2, иначе
в pipe1. Дочерние процессы инвертируют строки.
Читает из консоли, пишет в файл

Лаба 2: 
файлы: seq.c par.c
вариант 20 - «Дан массив координат (x, y, z). Необходимо найти три точки, которые образуют треугольник максимальной площади»

запуск: 
cd "/mnt/c/Users/Macherator/Desktop/универ/2_курс/лабы_опер_сист/2_лаба"
gcc -O3 -Wall -o seq seq.c -lm
gcc -O3 -Wall -pthread -o par par.c -lm
запуск последовательной 
./seq points.txt
запуск параллельной на 12 потоках 
./par 12 points.txt

Лаба 3:
файлы parent3.c child3.c
cd "/mnt/c/Users/Macherator/Desktop/универ/2_курс/лабы_опер_сист/3_лаба"
компиляция:
gcc -O3 -Wall -o parent3 parent3.c -pthread -lrt
gcc -O3 -Wall -o child3 child3.c -pthread -lrt
запуск:
./parent3
посмотреть результат:
cat out1.txt
cat out2.txt


