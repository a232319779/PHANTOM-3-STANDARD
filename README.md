# PHANTOM-3-STANDARD
Used hackrf one to sniffer PHANTOM 3 STANDARD RC signal

* **capture**抓包模块
	>Usage:  
	[-f freq\_hz] # Frequency in Hz [0MHz to 7250MHz].  
	[-a amp\_enable] # RX/TX RF amplifier 1=Enable, 0=Disable.  
	[-l gain\_db] # RX LNA (IF) gain, 0-40dB, 8dB steps.  
	[-g gain\_db] # RX VGA (baseband) gain, 0-62dB, 2dB steps.  
	[-s sample\_rate\_hz] # Sample rate in Hz (4/8/10/12.5/16/20MHz, default 1MHz).  
	[-n num\_samples] # Number of samples to transfer (default is unlimited).  
	[-r <filename>] # Receive data into file.  
    Default set : freq\_hz = 5738MHz, amp\_enable = 1, lna\_gain = 32, vga\_gain = 20, sample\_rate = 1MHz, num\_samples = 1*sample\_rate, filename = data/1M\_5738\_recive\_1s.iq

	示例：  
	载波频率5738MHz，启用天线，IF增益32db,BB增益20db,采样率1MHz,采样点1M,保存文件名data/1M\_5738\_recive\_1s.iq
    
	`$ ./capture -f 5738000000 -a 1 -l 32 -g 20 -s 1000000 -n 1000000 -r data/1M_5738_recive_1s.iq`
    
	或者直接运行
    
	`$ ./capture`	

* **decode**解码模块

	>Usage:  
	[-a] # preamble length [1 to 8].Default 1.  
	[-b] # preamble [1 to 8 bytes].Default '0xAA'.  
	[-c] # mac address [1 to 5].Default 5.  
	[-d] # if use esb [1 yes, 0 no].Default 1.  
	[-e] # pcf len. Default 2.  
	[-f] # crc len. Default 2.  
	[-x] # slot number. Default 16.  
	[-y] # period per signal.Deafult 7(ms).  
	[-z] # signal file. Default 'data/1M\_5738\_recive\_1s.iq'.  
	[-s] # sample rate. Deafult 1MHz.  
	[-h] # Display this text.  
	
	示例:  
	preamble长度1字节,preamble是0xAA,MAC地址长度5字节,使用esb模式,pcf长度2字节,crc长度2字节,包含16个时隙(计算周期),每个时隙长度7ms，信号文件'data/1M\_5738\_recive\_1s.iq',采样率1MHz
	
	`$ ./decode -a 1 -b 0xAA -c 5 -d 1 -e 2 -f 2 -x 16 -y 7 -z data/1M_5738_recive_1s.iq`
    
	或者直接运行
    
	`$ ./decode`
        
* **update**:

    * It default use 1MHz sample rate.(2017.02.22)  
    * It default use 4MHz sample rate.(before)
