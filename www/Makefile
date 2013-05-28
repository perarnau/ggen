PANDOC=/home/perarnau/.cabal/bin/pandoc

all: index.html bibtex.html

index.html: index.markdown template.html
	$(PANDOC) -s -f markdown -t html -c style.css --section-divs --template template -o $@ $<

bibtex.html: bibtex.markdown publi_bib.html template.html
	$(PANDOC) -f markdown -t html -c style.css --template template -A publi_bib.html -o $@ $<

publi_bib.html: publi.bib
	bibtex2html -nodoc publi.bib
	sed -i -e's/publi\.html//g' publi_bib.html
	rm publi.html
