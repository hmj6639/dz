########THIS MAKEFILE IS USED UNDER LINUX ONLY#######

build_dir=../build-dz-Desktop-Debug

all: compile run

compile:
	@mkdir -p $(build_dir)
	@qmake dz.pro -r CONFIG+=debug CONFIG+=qml_debug -o $(build_dir)/Makefile > /dev/null
	make -j4 -s -C $(build_dir)
	@echo ""
	@echo ""
	@echo "##############################################"
	@echo ""
	@echo ./.run.sh [config file]
	@echo ""
	@echo "##############################################"

run:
	chmod a+x .run.sh
	@./.run.sh

clean:
	@rm -rf $(build_dir)/*
	@find -name "*.pro.user" | xargs rm -f

fixtime:
	@rm -rf $(build_dir)
	@find . -exec touch {} \;
