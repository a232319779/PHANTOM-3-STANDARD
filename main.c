#include <memory.h>
#include "bk5811_demodu.h"

int main(int argc, char *argv[])
{
    char *sig_file = "4M_5743_recive_0.5_11_29.iq";
    if (argc == 2)
      sig_file = argv[1];

    get_signal_data(sig_file);
    for(long i = 0; i < g_file_length; i += PACKET_SIZE)
    {
        memset(g_inter, 0, sizeof(g_inter));
        mean(g_buffer, i, PACKET_SIZE);
        find_inter(g_buffer, i, PACKET_SIZE);
        work();
    }
    printf("end.\n");
    //release the memory.
    release();

    return 0;
}
