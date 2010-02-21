EXTRA_DIST = sectioncrc.py.c setup.py

pyflags = --prefix=${prefix} --exec-prefix=${exec_prefix}

all-local:
	${PYTHON24} setup.py build

clean-local:
	${PYTHON24} setup.py clean

install-exec-local:
	${PYTHON24} setup.py install ${pyflags}

