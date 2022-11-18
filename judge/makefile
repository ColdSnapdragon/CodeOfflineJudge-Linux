
ALL:run.exe

%.exe:%.c
	gcc $^ -o $@ -g -Wall -pthread -std=gnu99	

%:%.c
	gcc $^ -o $@ -g -Wall -pthread -std=gnu99	

clean:
	-rm -rf *.exe *.txt *.log Test
	
.PHONY:
	clean ALL
