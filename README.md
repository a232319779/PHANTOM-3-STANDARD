# PHANTOM-3-STANDARD
used hackrf one to sniffer PHANTOM 3 STANDARD RC signal

version 1.0
    Executable files:
        capture : Used to capture the signal and save to disk. Such as "capture -f 5743000000 -a 1 -l 40 -g 20 -s 4000000 -n 2000000 -b 1000000 -r 4M_5743_recive_0.5.iq"
        decode : Used to decode the signal file.
        scan_phantom : Scan the phantom signal real-time.it could scan all 125 channles and get the period.But couldn't get the hopping pattern.
    
    update:
        It default use 4MHz sample rate.(before)
        It default use 1MHz sample rate.(2017.02.22)
