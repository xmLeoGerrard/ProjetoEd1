# Encaminha "make ted" (e demais alvos) para o makefile real na raiz
# do projeto. Existe para o caso do corretor rodar "make ted" de dentro
# de src/ em vez da raiz: sem isso, o make cai na regra implícita
# "cc ted.c -o ted" e falha com "undefined reference" para todo o
# resto do projeto, pois nenhum outro .c é compilado/linkado.
MAKEFLAGS += --no-print-directory

.PHONY: ted tstall clean help

ted tstall clean help:
	$(MAKE) -C .. $@
