build_frontend:
	$(MAKE) -C ./frontend

clean_frontend:
	$(MAKE) -C ./frontend clean

test_frontend:
	$(MAKE) -C ./frontend test

clean: clean_frontend

distclean:
	$(MAKE) -C ./frontend distclean
