OBJECT=./BigInt/BigInt.o
BigIntSer.cgi:$(OBJECT) BigIntSer.o
	g++ -o BigIntSer.cgi BigIntSer.o $(OBJECT) -g
BigIntSer.o:BigIntSer.cpp 
	g++ -o BigIntSer.o -c BigIntSer.cpp -g
clean:
	rm BigIntSer.cgi *.o
