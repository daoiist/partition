#include "postPartition.h"

int main()
{
	postPartition pp1("zl_age");
	postPartition pp2("zl_sex");
	postPartition pp12("zl_age","zl_sex");
	std::map<std::string,std::set<int>> GB1=pp1.getGB();
	std::map<std::string,std::set<int>> GB12=pp12.getGB();





	return 0;
}
