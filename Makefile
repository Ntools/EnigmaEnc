all:
	make -C src
	make -C src/win-cgi

install:
	make -C src install
	make -C src/win-cgi install

clean:	
	make -C src clean
	make -C src/win-cgi clean
