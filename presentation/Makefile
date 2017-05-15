TARGET="source"
TEX="${TARGET}.tex"
PDF="${TARGET}.pdf"
AUX="${TARGET}.aux"

all:
	pdflatex ${TEX}

bibtex:
	latex ${TEX}
	bibtex ${AUX}
	latex ${TEX}
	latex ${TEX}
	pdflatex ${TEX}

open:
	open ${PDF}

clean:
	rm *.aux *.log *.blg *.out *.toc *.bbl *.dvi
