clean:
	-astyle -r "*.hpp" "*.cpp" "*.h"
	find . -type f -name '*.orig' -delete