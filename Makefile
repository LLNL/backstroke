all:
	$(MAKE) -C src

install: all
	$(MAKE) -C src install

uninstall:
	$(MAKE) -C src uninstall

check: src/backstroke
	$(MAKE) -C tests check

check-extended: src/backstroke
	$(MAKE) -C tests check-extended

docs:
	$(MAKE) -C docs

clean:
	$(MAKE) -C src clean
	$(MAKE) -C tests clean
	$(MAKE) -C docs clean

src/backstroke: src/backstroke.C
	$(MAKE) -C src
