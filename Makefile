.PHONY: run
run:
	flex test.l
	bison -vdty calc.y
	gcc lex.yy.c y.tab.c
	./a.out
