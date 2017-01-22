#include <memory.h>
#include "bk5811_demodu.h"

int main(int argc, char *argv[])
{
    char *sig_file = "4M_5743_recive_0.5_11_29.iq";
    if (argc == 2)
      sig_file = argv[1];

    char *buffer = NULL;
    long file_length = 0;

    get_signal_data(sig_file, &buffer, &file_length);
    
    memset(g_inter, 0, sizeof(g_inter));
    mean(buffer, 0, file_length);
    find_inter(buffer, 0, file_length);
    work(buffer, NULL);

    printf("end.\n");
    //release the memory.
    release(buffer);

    return 0;
}
