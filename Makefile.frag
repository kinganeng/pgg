pgg: $(SAPI_PGG_PATH)

$(SAPI_PGG_PATH): $(PHP_GLOBAL_OBJS) $(PHP_BINARY_OBJS) $(PHP_PGG_OBJS)
	$(BUILD_PGG)

install-pgg: $(SAPI_PGG_PATH)
	@echo "Installing PHP PGG binary:        $(INSTALL_ROOT)$(sbindir)/"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(sbindir)
	@$(mkinstalldirs) $(INSTALL_ROOT)$(localstatedir)/log
	@$(mkinstalldirs) $(INSTALL_ROOT)$(localstatedir)/run
	@$(INSTALL) -m 0755 $(SAPI_PGG_PATH) $(INSTALL_ROOT)$(sbindir)/$(program_prefix)php-pgg$(program_suffix)$(EXEEXT)

	@echo "Installing PHP PGG config:        $(INSTALL_ROOT)$(sysconfdir)/" && \
	$(mkinstalldirs) $(INSTALL_ROOT)$(sysconfdir) || :
	@$(INSTALL_DATA) sapi/pgg/php-pgg.conf $(INSTALL_ROOT)$(sysconfdir)/php-pgg.conf.default || :

	@echo "Installing PHP PGG man page:      $(INSTALL_ROOT)$(mandir)/man8/"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(mandir)/man8
	@$(INSTALL_DATA) sapi/pgg/php-pgg.8 $(INSTALL_ROOT)$(mandir)/man8/php-pgg$(program_suffix).8
