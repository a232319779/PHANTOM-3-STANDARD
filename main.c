#include "bk5811_demodu.h"

int main(int argc, char *argv[])
{
    char *sig_file = "4M_5743_recive_0.5_11_29.iq";
    if (argc == 2)
      sig_file = argv[1];

    get_signal_data(sig_file);
    mean(g_buffer, g_file_length);
    find_inter(g_buffer, g_file_length);
    work();

    //release the memory.
    release();

    return 0;
}
