
define testbinary
	@echo "---------------------------"
	@./$(1)
	@echo "---------------------------"
	@echo "Testing addr2line for $(1)"
	$(eval REL_ADDR = $(shell ./$(1) | grep -o "rel:\ 0[x][0-9a-f]\+" | cut -d' ' -f2))
	addr2line -e ./$(1) ${REL_ADDR} -f

endef

all:
	gcc -no-pie -rdynamic -g -pg -finstrument-functions main.c ext.c -ldl -o main7
	gcc -rdynamic -g -pg -finstrument-functions main.c ext.c -ldl -o main7-pie
	$(call testbinary,main7)
	$(call testbinary,main7-pie)
