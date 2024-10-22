Running Instructions

To run this project, first create a 2GB partition (formatted as ext4) on your SSD for testing. Make note of the name of this partition (in my case, it was /dev/nvme0n1p6). Next install the Flexible IO Tester (FIO) and download the ssd_test.c file from the repository. Within this file, you should change the “--filename=/dev/nvme0n1p6 argument on line 11 to match your partition name. Now you can compile with gcc -o ssd_test ssd_test.c and run it (while capturing the output data) via ./ssd_test > output_log.txt. (This txt file is included in the repository for viewing).

This program executes 64 FIO test commands, testing all combinations of block_sizes [4k, 16k, 32k, 128k], R/W ratios [100, 0, 50, 70]. The I/O depths [1, 32, 256, 1024]. The output log can be parsed to find the IOPS, bandwidth, and latency of each test. The resulting data and generated graphs can be seen in the google sheet at:
https://docs.google.com/spreadsheets/d/1lgxt_eIk4_H00SV9GrEZ2uW2mJoXc-iMJnGU6dBBGMc/edit?usp=sharing

Formal analysis of these graphs can be seen in the PDF

Note: The SSD used for these tests was a WDC PC SN730 SDBQNTY-512-1001.

