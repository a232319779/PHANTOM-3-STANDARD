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
    long end = 0;
    for(long i = 0; i < file_length; i += PACKET_SIZE)
    {
        end = PACKET_SIZE < (file_length - i) ? PACKET_SIZE : (file_length - i - 2);
        memset(g_inter, 0, sizeof(g_inter));
        mean(buffer, i, end);
        find_inter(buffer, i, end);
        work(buffer);
    }
    printf("end.\n");
    //release the memory.
    release(buffer);

    return 0;
}
