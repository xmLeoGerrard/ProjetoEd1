# ── Makefile do Projeto TED ────────────────────────────────────────
PROJ_NAME = ted
ALUNO     = Leonardo Gerrard

CC      = gcc
CFLAGS  = -std=c99 -fstack-protector-all -ggdb -O0 -Wall -Wextra \
          -Werror=implicit-function-declaration \
          -Isrc -Iunity
LDFLAGS = -O0 -lm

# ── Objetos principais ─────────────────────────────────────────────
OBJETOS = src/hashfile.o \
          src/quadra.o   \
          src/pessoa.o   \
          src/svg.o      \
          src/args.o     \
          src/cidade.o   \
          src/parser.o   \
          src/ted.o

UNITY_OBJ = unity/unity.o

# ── Executável principal ───────────────────────────────────────────
$(PROJ_NAME): $(OBJETOS)
	$(CC) -o src/$(PROJ_NAME) $(LDFLAGS) $(OBJETOS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ── Dependências ───────────────────────────────────────────────────
src/ted.o:      src/ted.c      src/args.h src/hashfile.h src/quadra.h \
                               src/pessoa.h src/parser.h src/svg.h src/cidade.h
src/hashfile.o: src/hashfile.c src/hashfile.h
src/quadra.o:   src/quadra.c   src/quadra.h src/hashfile.h
src/pessoa.o:   src/pessoa.c   src/pessoa.h src/hashfile.h
src/svg.o:      src/svg.c      src/svg.h
src/args.o:     src/args.c     src/args.h
src/cidade.o:   src/cidade.c   src/cidade.h src/hashfile.h src/quadra.h \
                               src/pessoa.h src/svg.h
src/parser.o:   src/parser.c   src/parser.h src/hashfile.h src/quadra.h \
                               src/pessoa.h src/svg.h src/cidade.h
unity/unity.o:  unity/unity.c  unity/unity.h

# ── Testes unitários ───────────────────────────────────────────────
TST_COMMON = unity/unity.o

# hashfile
tst/t_hashfile: tst/t_hashfile.o src/hashfile.o $(TST_COMMON)
	$(CC) -o $@ $(LDFLAGS) $^

tst/t_hashfile.o: tst/t_hashfile.c src/hashfile.h unity/unity.h
	$(CC) $(CFLAGS) -c $< -o $@

tst_hashfile: tst/t_hashfile
	./tst/t_hashfile

# quadra
tst/t_quadra: tst/t_quadra.o src/quadra.o src/hashfile.o $(TST_COMMON)
	$(CC) -o $@ $(LDFLAGS) $^ -lm

tst/t_quadra.o: tst/t_quadra.c src/quadra.h src/hashfile.h unity/unity.h
	$(CC) $(CFLAGS) -c $< -o $@

tst_quadra: tst/t_quadra
	./tst/t_quadra

# pessoa
tst/t_pessoa: tst/t_pessoa.o src/pessoa.o src/hashfile.o $(TST_COMMON)
	$(CC) -o $@ $(LDFLAGS) $^

tst/t_pessoa.o: tst/t_pessoa.c src/pessoa.h src/hashfile.h unity/unity.h
	$(CC) $(CFLAGS) -c $< -o $@

tst_pessoa: tst/t_pessoa
	./tst/t_pessoa

# args
tst/t_args: tst/t_args.o src/args.o $(TST_COMMON)
	$(CC) -o $@ $(LDFLAGS) $^

tst/t_args.o: tst/t_args.c src/args.h unity/unity.h
	$(CC) $(CFLAGS) -c $< -o $@

tst_args: tst/t_args
	./tst/t_args

# cidade
tst/t_cidade: tst/t_cidade.o src/cidade.o src/hashfile.o src/quadra.o \
               src/pessoa.o src/svg.o $(TST_COMMON)
	$(CC) -o $@ $(LDFLAGS) $^

tst/t_cidade.o: tst/t_cidade.c src/cidade.h src/hashfile.h src/quadra.h \
                src/pessoa.h src/svg.h unity/unity.h
	$(CC) $(CFLAGS) -c $< -o $@

tst_cidade: tst/t_cidade
	./tst/t_cidade

# ── tstall: compila e executa todos os testes ──────────────────────
tstall: tst/t_hashfile tst/t_quadra tst/t_pessoa tst/t_args tst/t_cidade
	@echo "=============================="
	@echo " Executando t_hashfile"
	@echo "=============================="
	./tst/t_hashfile
	@echo "=============================="
	@echo " Executando t_quadra"
	@echo "=============================="
	./tst/t_quadra
	@echo "=============================="
	@echo " Executando t_pessoa"
	@echo "=============================="
	./tst/t_pessoa
	@echo "=============================="
	@echo " Executando t_args"
	@echo "=============================="
	./tst/t_args
	@echo "=============================="
	@echo " Executando t_cidade"
	@echo "=============================="
	./tst/t_cidade
	@echo "=============================="
	@echo " TODOS OS TESTES CONCLUIDOS"
	@echo "=============================="

# ── Limpeza ────────────────────────────────────────────────────────
clean:
	rm -f src/*.o unity/*.o tst/*.o
	rm -f src/$(PROJ_NAME)
	rm -f tst/t_hashfile tst/t_quadra tst/t_pessoa tst/t_args tst/t_cidade

.PHONY: tstall tst_hashfile tst_quadra tst_pessoa tst_args tst_cidade clean
