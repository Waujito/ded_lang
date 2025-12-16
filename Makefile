.PHONY: build clean distclean 
build: build_shared build_frontend build_middleend build_backend

.PHONY: build_shared clean_shared
build_shared:
	$(MAKE) -C ./shared

clean_shared:
	$(MAKE) -C ./shared clean

.PHONY: build_frontend test_frontend

build_frontend:
	$(MAKE) -C ./frontend

clean_frontend:
	$(MAKE) -C ./frontend clean

test_frontend:
	$(MAKE) -C ./frontend test

.PHONY: build_middleend test_middleend clean_middleend

build_middleend:
	$(MAKE) -C ./middleend

clean_middleend:
	$(MAKE) -C ./middleend clean

test_middleend:
	$(MAKE) -C ./middleend test

.PHONY: build_backend test_backend clean_backend

build_backend:
	$(MAKE) -C ./backend

clean_backend:
	$(MAKE) -C ./backend clean

test_backend:
	$(MAKE) -C ./backend test

clean:
	$(MAKE) -C ./frontend	clean
	$(MAKE) -C ./middleend	clean
	$(MAKE) -C ./shared	clean
	$(MAKE) -C ./backend	clean

distclean:
	$(MAKE) -C ./frontend	distclean
	$(MAKE) -C ./middleend	distclean
	$(MAKE) -C ./shared	distclean
	$(MAKE) -C ./backend	distclean
