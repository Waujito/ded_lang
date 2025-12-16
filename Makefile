.PHONY: build build_shared build_frontend test_frontend clean distclean

build: build_frontend build_shared build_middleend

build_shared:
	$(MAKE) -C ./shared

clean_shared:
	$(MAKE) -C ./shared clean

build_frontend:
	$(MAKE) -C ./frontend

clean_frontend:
	$(MAKE) -C ./frontend clean

test_frontend:
	$(MAKE) -C ./frontend test

build_middleend:
	$(MAKE) -C ./middleend

clean_middleend:
	$(MAKE) -C ./middleend clean

test_middleend:
	$(MAKE) -C ./middleend test

clean:
	$(MAKE) -C ./frontend	clean
	$(MAKE) -C ./middleend	clean
	$(MAKE) -C ./shared	clean

distclean:
	$(MAKE) -C ./frontend	distclean
	$(MAKE) -C ./middleend	distclean
	$(MAKE) -C ./shared	distclean
