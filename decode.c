#include <memory.h>
#include "bk5811_demodu.h"

int main(int argc, char *argv[])
{
    char *sig_file = "data/4M_5743_recive_0.5_11_29.iq";
    if (argc == 2)
      sig_file = argv[1];

    char *buffer = NULL;
    long file_length = 0;
    long start_position = -1;
    uint8_t channel = 0;

    get_signal_data(sig_file, &buffer, &file_length);
    
    memset(g_inter, 0, sizeof(g_inter));
    mean(buffer, 0, file_length);
    find_inter(buffer, 0, file_length);
    //set_inter(file_length);
    work(buffer, &start_position, &channel);

    printf("end.\n");
    //release the memory.
    release(buffer);

    return 0;
}
