all:
	make -C src/generator/
	cd doc && \
		find ./* -type d \
			-exec mkdir -p "../html/{}" \; \
			-exec cp ../style.css "../html/{}" \; && \
		cp ../style.css ../html/ && \
		find . -name "*.txt" \
			-exec sh -c '../src/generator/genhtml < "{}" > "../html/$$($$1)/$$($$2).html"' sh 'dirname {}' 'basename -s .txt {}' \;
