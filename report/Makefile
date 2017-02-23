all:
	pdflatex report.tex

bibtex:
	latex report.tex
	bibtex report.aux
	latex report.tex
	latex report.tex
	pdflatex report.tex

open:
	open report.pdf

clean:
	rm *.aux *.log *.blg *.out *.toc *.bbl *.dvi
