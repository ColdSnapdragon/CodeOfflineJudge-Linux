
ALL:run.exe

%.exe:%.c
	gcc $^ -o $@ -g -Wall -pthread -std=gnu99	

%:%.c
	gcc $^ -o $@ -g -Wall -pthread -std=gnu99	

clean:
	-rm -rf *.exe
	
.PHONY:
	clean ALL
