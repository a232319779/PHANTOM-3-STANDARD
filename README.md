# PHANTOM-3-STANDARD
## Used hackrf one to sniffer PHANTOM 3 STANDARD RC signal

* **capture**抓包模块  
基于hackrf_transfer.c源码，只保留了必需的参数。只能指定一个载波频率抓包(默认设置针对大疆3标准版抓包)
	>Usage:  
	[-h help] # Display this text.  
	[-f freq\_hz] # Frequency in Hz [0MHz to 7250MHz].  
	[-a amp\_enable] # RX/TX RF amplifier 1=Enable, 0=Disable.  
	[-l gain\_db] # RX LNA (IF) gain, 0-40dB, 8dB steps.  
	[-g gain\_db] # RX VGA (baseband) gain, 0-62dB, 2dB steps.  
	[-s sample\_rate\_hz] # Sample rate in Hz (4/8/10/12.5/16/20MHz, default 1MHz).  
	[-n num\_samples] # Number of samples to transfer (default is unlimited).  
	[-r <filename>] # Receive data into file (use '-' for stdout).  
Default set : -f 5738000000 -a 1 -l 32 -g 20 -s 1000000 -n 1000000 -r data/1M\_5738\_recive\_1s.iq

	参数解释：  
	载波频率5738MHz，启用外部放大器，IF增益32db,BB增益20db,采样率1MHz,采样点1M,保存文件名data/1M\_5738\_recive\_1s.iq
    
	`$ ./capture -f 5738000000 -a 1 -l 32 -g 20 -s 1000000 -n 1000000 -r data/1M_5738_recive_1s.iq`
    
	或者直接运行
    
	`$ ./capture`	
	
	执行结果如图所示:  
	![capture](https://github.com/a232319779/PHANTOM-3-STANDARD/blob/master/screenshot/capture.png)

* **decode**解码模块  
对离线数据包进行解调和解析。参数包括preamble长度和值,MAC地址长度,是否是ESB模式,PCF长度,CRC长度,时隙个数,时隙时长,需要解码对数据文件,数据文件的采样率。(默认设置针对大疆3标准版解码)。

	>Usage:  
	[-h] # Display this text.  
	[-i] # preamble length [1 to 8].Default 1.  
	[-j] # preamble [1 to 8 bytes].Default '0xAA'.  
	[-m] # mac address [1 to 5].Default 5.  
	[-e] # if use esb [1 yes, 0 no].Default 1.  
	[-p] # pcf len. Default 2.  
	[-c] # crc len. Default 2.  
	[-t] # slot number. Default 16.  
	[-y] # period per signal.Deafult 7(ms).  
	[-r] # signal file. Default 'data/1M\_5738\_recive\_1s.iq'.  
	[-s] # sample rate. Deafult 1MHz.  
Default set : -i 1 -j 0xAA -m 5 -e 1 -p 2 -c 2 -t 16 -y 7 -s 1000000 -r data/1M\_5738\_recive\_1s.iq
	
	参数解释:  
	preamble长度1字节,preamble是0xAA,MAC地址长度5字节,使用esb模式,pcf长度2字节,crc长度2字节,包含16个时隙(计算周期),每个时隙长度7ms，信号文件'data/1M\_5738\_recive\_1s.iq',采样率1MHz
	
	`$ ./decode -i 1 -j 0xAA -n 5 -e 1 -p 2 -c 2 -t 16 -y 7 -r data/1M_5738_recive_1s.iq -s 1000000`
    
	或者直接运行
    
	`$ ./decode`
	
	执行结果如图所示:  
	![decode](https://github.com/a232319779/PHANTOM-3-STANDARD/blob/master/screenshot/decode.png)
	
* **scan_phantom**扫描模块  
实时扫描指定载波频率开始的n个信道,每个信道带宽1M.扫描完成之后计算出'时隙'大小，'突发'大小.

	>Usage:  
	[-h] # Display this text.  
	[-i] # Preamble length [1 to 8].Default 1.  
	[-j] # Preamble [1 to 8 bytes].Default '0xAA'.  
	[-m] # Mac address [1 to 5].Default 5.  
	[-e] # If use esb [1 yes, 0 no].Default 1.  
	[-p] # Pcf len. Default 2.  
	[-c] # Crc len. Default 2.  
	[-q] # Channles count, should less 200. Default 127.  
	[-S] # Size per channel. Default 1.  
	[-f] # Start frequency in Hz [1MHz to 6000MHz].  
	[-a] # RX/TX RF amplifier 1=Enable, 0=Disable.  
	[-l] # RX LNA (IF) gain, 0-40dB, 8dB steps.  
	[-g] # RX VGA (baseband) gain, 0-62dB, 2dB steps.  
	[-s] # Sample rate in Hz (4/8/10/12.5/16/20MHz, default 1MHz).  
Default set : -i 1 -j 0xAA -m 5 -e 1 -p 2 -c 2 -q 127 -S 1 -f 5725000000 -a 1 -l 32 -g 20 -s 1000000

	参数解释:
	preamble长度1字节,preamble是0xAA,MAC地址长度5字节,使用esb模式,pcf长度2字节,crc长度2字节,扫描信道数目127,每个信道采大小1MHz,起始载波频率5725MHz,启用外部放大器,中频增益32db,基带增益20db,采样率1MHz
	
	`$ ./scan_phantom -i 1 -j 0xAA -m 5 -e 1 -p 2 -c 2 -q 127 -S 1 -f 5725000000 -a 1 -l 32 -g 20 -s 1000000`
	
	或者直接运行
	
	`$ ./scan_phantom`

	执行结果如图所示:  
	![scan_phantom](https://github.com/a232319779/PHANTOM-3-STANDARD/blob/master/screenshot/scan_phantom.png)

* **calc_hopping**计算跳频图案模块
	* 开发中  

## **update**:
* Suport set args.(2017.03.14)
* It default use 1MHz sample rate.(2017.02.22)  
* It default use 4MHz sample rate.(before)
