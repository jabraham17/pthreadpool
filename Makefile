
TOPTARGETS= all clean

SUBDIRS= pthreadpool src/bench src/test


.PHONY: $(TOPTARGETS)

$(TOPTARGETS): $(SUBDIRS)

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) --directory=$@ $(MAKECMDGOALS)
