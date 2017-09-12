#include "engine.h"

int			main()
{
	VkHandler		k3Handler;

	try {
		k3Handler.run();
	}
	catch (const std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
		k3Handler.terminate();
	}
	return EXIT_SUCCESS;
}