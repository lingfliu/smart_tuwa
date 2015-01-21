#include "twrt.h"

int main(int argn, char* argv[]){

    get_config(&cfg);
    //config_app = get_config();
    printf("%s\n",cfg.inet_server_ip);
    return 0;
}
