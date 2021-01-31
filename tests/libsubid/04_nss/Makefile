all: test_nss libsubid_zzz.so

test_nss: test_nss.c ../../../libmisc/nss.c
	gcc -I../../../lib/ -I../../.. -o test_nss test_nss.c ../../../libmisc/nss.c -ldl

libsubid_zzz.so: libsubid_zzz.c
	gcc -c -I../../../lib/ -I../../.. -I../../../libmisc -I../../../libsubid libsubid_zzz.c
	gcc -L../../../libsubid -shared -o libsubid_zzz.so libsubid_zzz.o

clean:
	rm -f *.o *.so test_nss
