#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <fstream>
#include <cstring>

#include "lm.h"
using namespace std;

int main(int argc, char* argv[]){
	std::ios_base::sync_with_stdio(false);
	#ifdef __APPLE__
	    //freopen("sample-all-cmds.txt", "r", stdin);
	    //freopen("tout", "w", stdout);
	#endif

	if(argc != 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0){
        cout << " --help or -h\n"
             << "    Prints this input specification.\n"
             << " LOGFILE\n"
             << "   Input file.\n";
        exit(0);
    }

    ifstream logstr;
    logstr.open(argv[1]);
    lm log_manager;
    
    log_manager.load_logfile(logstr);
    log_manager.calculation();
    
    logstr.close();

}
