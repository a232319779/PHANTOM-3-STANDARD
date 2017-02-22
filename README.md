# PHANTOM-3-STANDARD
Used hackrf one to sniffer PHANTOM 3 STANDARD RC signal

    Executable files:
        capture : Used to capture the signal and save to disk.
                  Default such as "capture -f 5743000000 -a 1 -l 40 -g 20 -s 1000000 -n 1000000 -b 1000000 -r data/1M_5743_recive_1s.iq".
                  For PHANTOM 3 STANDARD, it could capture 9 signal in 1 seconds.
        decode : Used to decode the signal file.
        scan_phantom : Scan the phantom signal real-time.it could scan all 125 channles and get the period.
        calc_hopping : try to describe the frequency hopping pattern.Not ok.
        
    update:
        It default use 4MHz sample rate.(before)
        It default use 1MHz sample rate.(2017.02.22)
