all: lib_test
	echo Done!
lib_test: lib_test.o
	gcc -g -o lib_test lib_test.o -L./ -lmy
lib_test.o: libmy.a lib_test.c
	gcc -g -c lib_test.c
libmy.a: file1.o file2.o
	ar rscv libmy.a file1.o file2.o
filex: file1.c file2.c
	gcc -g -c file1.c file2.c

clean:
	rm lib_test.o lib_test file1.o file2.o libmy.a
