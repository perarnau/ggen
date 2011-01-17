.1md.1:
	pandoc -s -t man $< -o $@
.3md.3:
	pandoc -s -t man $< -o $@

man1_MANS = ggen.1

man3_MANS = ggen.3

mostlyclean-local:
	-rm $(man1_MANS) $(man3_MANS)

EXTRA_DIST = $(man1_MANS) $(man3_MANS)
